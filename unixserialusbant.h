#ifndef UNIXSERIALUSBANT_H
#define UNIXSERIALUSBANT_H

#include <QByteArray>
#include <QFileInfo>
#include <QFileInfoList>
#include <QObject>
#include <QStringList>

class UnixSerialUsbAnt : public QObject
{
	Q_OBJECT
public:
	explicit UnixSerialUsbAnt(QObject *parent = 0);
	virtual ~UnixSerialUsbAnt();
	/** Find a Garmin USB1 Stick. If found, returns true, false otherwise */
	static bool isAntUsb1StickPresent();

	bool isValid();

	int writeBytes(QByteArray& bytes);
	QByteArray readBytes();
signals:
	
public slots:
private:
	static QFileInfoList findUsbSerialDevices();
	static bool isGarminUsb1Stick(const QFileInfo& fileInfo);

	int openConnection();

	QFileInfo _deviceFileInfo;
	int _nativeDeviceHandle;
};

#endif // UNIXSERIALUSBANT_H
