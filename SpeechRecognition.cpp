#include "SpeechRecognition.h"

#include <QDebug>

#include <pocketsphinx.h>
#include <ad.h>
#include <cont_ad.h>

const arg_t DEFAULT_ARGUMENTS[] = {
	POCKETSPHINX_OPTIONS,
	/* Argument file. */
	{ "-argfile",
	  ARG_STRING,
	  NULL,
	  "Argument file giving extra arguments." },
	{ "-adcdev",
	  ARG_STRING,
	  NULL,
	  "Name of audio device to use for input." },
	{ "-infile",
	  ARG_STRING,
	  NULL,
	  "Audio file to transcribe." },
	{ "-time",
	  ARG_BOOLEAN,
	  "no",
	  "Print word times in file transcription." },
	CMDLN_EMPTY_OPTION
};

SpeechRecognition::SpeechRecognition(QObject *parent) :
	QThread(parent), m_running(true), m_decoder(NULL)
{
}

SpeechRecognition::~SpeechRecognition()
{
	if(m_decoder != NULL)
		ps_free(m_decoder);
}

bool SpeechRecognition::init(const QString &device)
{
	m_config = cmd_ln_parse_r(NULL, DEFAULT_ARGUMENTS, 0, NULL, FALSE);

	if(m_config == NULL)
		return false;

	m_decoder = ps_init(m_config);

	if(m_decoder == NULL)
		return false;

	return true;
}

void SpeechRecognition::run()
{
	ad_rec_t *device = NULL;
	cont_ad_t *continuous = NULL;
	qint16 buffer[4096];
	qint32 length, remaining, timestamp;
	char const* uttid;
	char const* hyp;
	QString decodedData;

	try
	{
		device = ad_open_dev(cmd_ln_str_r(m_config, "-adcdev"),
							 (int)cmd_ln_float32_r(m_config, "-samprate"));

		if(device == NULL)
			throw QString( "Audio device not open");

		continuous = cont_ad_init(device, ad_read);

		if(continuous == NULL)
			throw QString("Could not init audio device in continuous mode");

		if(ad_start_rec(device) < 0)
			throw QString("Could not start recording on device");

		if(cont_ad_calib(continuous) << 0)
			throw QString("Could not calibrate device");

		// main recognition loop
		while(m_running)
		{
			// wait for data
			while((length = cont_ad_read(continuous, buffer, 4096)) == 0)
				msleep(50);

			if(length < 0)
				throw QString("Error while reading from microphone");

			qDebug() << "Listening";


			ps_start_utt(m_decoder, NULL);
			timestamp = continuous->read_ts;
			ps_process_raw(m_decoder, buffer, length, FALSE, FALSE);

			// decoding utterance
			while(m_running)
			{
				length = cont_ad_read(continuous, buffer, 4096);

				if(length < 0)
					throw QString("Could not read data  from device");
				else if(length == 0)
				{
					if((continuous->read_ts - timestamp) > DEFAULT_SAMPLES_PER_SEC/4)
						break;
				}
				else
				{
					timestamp = continuous->read_ts;
				}

				remaining = ps_process_raw(m_decoder, buffer, length, FALSE, FALSE);

				if((remaining == 0) && (length == 0))
					msleep(20);
			}

			ad_stop_rec(device);
			while(ad_read(device, buffer, 4096) >= 0);
			cont_ad_reset(continuous);
			ps_end_utt(m_decoder);
			decodedData = QString(ps_get_hyp(m_decoder, NULL, &uttid));

			qDebug() << "Decoded:" << decodedData;

			if(decodedData.contains("full screen"))
				emit fullscreen();
			else if(decodedData.contains("fire"))
				emit fire();

			if(ad_start_rec(device) < 0)
				throw QString("Could not resume recording");

		}
	}
	catch(QString error)
	{
		qDebug() << error;
	}

	if(continuous != NULL)
		cont_ad_close(continuous);

	if(device != NULL)
		ad_close(device);
}

void SpeechRecognition::stop()
{
	m_running = false;
}
