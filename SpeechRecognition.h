#ifndef SPEECHRECOGNITION_H
#define SPEECHRECOGNITION_H

#include <QThread>

typedef struct cmd_ln_s cmd_ln_t;
typedef struct ps_decoder_s ps_decoder_t;

class SpeechRecognition : public QThread
{
	Q_OBJECT
public:
	explicit SpeechRecognition(QObject *parent = 0);
	~SpeechRecognition();

	bool init(const QString &device);

	void stop();

signals:
	void fullscreen();
	void fire();

protected:
	virtual void run();

private:
	bool m_running;

	ps_decoder_t *m_decoder;
	cmd_ln_t *m_config;
};

#endif // SPEECHRECOGNITION_H
