#ifndef PSMOVEFORM_H
#define PSMOVEFORM_H

#include "MoveData.h"

#include <QWidget>
#include <QMatrix4x4>

namespace Ui {
	class PSMoveForm;
}

class QCheckBox;
class QLabel;
class QStackedLayout;
class IirFilter;

class PSMoveForm : public QWidget
{
	Q_OBJECT

public:
	explicit PSMoveForm(QWidget *parent = 0);
	~PSMoveForm();

public slots:
	void setButtons(MoveButtons buttons);
	void parseMoveData(MoveData d);

signals:
	void setRgb(QColor rgb);
	void setRumble(quint8 rumble);
	void setVector(QVector3D vec);
	void setMatrix(QMatrix4x4 vec);
	void setTopRightCorner();
	void setBottomLeftCorner();
	void movePressed();

private slots:
	void on_redDial_valueChanged(int value);

	void on_greenDial_valueChanged(int value);

	void on_blueDial_valueChanged(int value);

private:
	Ui::PSMoveForm *ui;
	QCheckBox *mBatteryCheckBoxes;
	QLabel *mBatteryChargingLabel;
	QStackedLayout *mBatteryLayout;
	QWidget *mBatteryPageWidgets;
	QColor mRGB;
	QVector3D mOne, mTwo, mThree;
	bool mPrevMovePressed, mPrevTriPressed, mPrevSquPressed;
	IirFilter *mAccFilters[3];
	IirFilter *mMagFilters[3];
	MoveData mPrevMoveData;
};

#endif // PSMOVEFORM_H
