#ifndef VIRTUALTRAININGFILEPARSER_H
#define VIRTUALTRAININGFILEPARSER_H

#include "model/reallifevideo.h"
#include <memory>
#include <QtCore/QObject>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QXmlStreamReader>

namespace indoorcycling {

namespace virtualtrainingfileparser {
struct ProfileEntry;
struct DistanceMappingEntry;
}

/**
 * RealLifeVideo parser for Cycleops Virtual Training (https://www.cycleops.com/virtualtraining/overview) files.
 *
 * Note that the file is in xml, and the header says it's utf-8. The files from reallifevideo.de
 * are not utf-8, but "Little-endian UTF-16 Unicode text". To cope with this, we do not open the
 * files directly, but use a QTextStream to read the file. This class will check the file's BOM
 * and read it to a QString in a proper way.
 *
 * header of a relevant Virtual Training .xml file (FR_Restonica):

<?xml version="1.0" encoding="utf-8" standalone="yes"?>

<kwt>
   <name>FR_Restonica</name>
   <id>ff466b5e7d6143759094b5d108903e7e</id>
   <video-file-path>FR_Restonica.avi</video-file-path>
  <title de="Georges de la Restonica" en="Georges de la Restonica" />
  <description1 de="Die Strecke f�hrt durch die bekannte Restonica Schlucht in Richtung Lac de M�lo." en="The route runs through the famous Restonica Canyon towards Lac de M�lo." />
  <description2 de="Mehr Real Life Videos auf www.reallifevideo.de" en="More Real Life Videos at www.reallifevideo.de" />
  <tree de="Reallifevideo.de/Frankreich" en="Reallifevideo.de/France" />
  <framerate>25.0</framerate>
  <start-frame>0</start-frame>
  <end-frame>37838</end-frame>
After this part, we get the list of altitude points:

 <altitudes>
                <altitude distance="0" height="487.96478558933774" />
                <altitude distance="20" height="488.1219320051037" />
                <altitude distance="34" height="488.2475545789071" />
                <altitude distance="50" height="488.4128496807195" />
                <altitude distance="68" height="488.60460257185514" />
                <altitude distance="85" height="488.8290764874856" />
                <altitude distance="103" height="489.13090988428456" />
                <altitude distance="121" height="489.5076425908948" />
                <altitude distance="141" height="489.96230585620094" />
                <altitude distance="160" height="490.4937270319016" />
                <altitude distance="177" height="491.0959861798757" />
                <altitude distance="194" height="491.7602464761807" />
                <altitude distance="210" height="492.4749542348527" />
                <altitude distance="225" height="493.2271831394758" />
         ...
</altitudes>
Then the list of frame-distance mappings:

<mappings>
                <mapping frame="0" distance="0" />
                <mapping frame="150" distance="34" />
                <mapping frame="250" distance="68" />
                <mapping frame="350" distance="103" />
                <mapping frame="450" distance="141" />
                <mapping frame="550" distance="177" />
                <mapping frame="650" distance="210" />
                <mapping frame="750" distance="242" />
                <mapping frame="850" distance="276" />
                <mapping frame="950" distance="313" />
                <mapping frame="1050" distance="351" />
                <mapping frame="1150" distance="389" />
                <mapping frame="1200" distance="409" />
                <mapping frame="1250" distance="429" />
          ...
</mappings>
a list of positions (mapped from distance):

<positions>
                <position distance="0" lat="42.2892107907683" lon="9.12955694831908" />
                <position distance="20" lat="42.2892192564905" lon="9.12931906990707" />
                <position distance="34" lat="42.2892150655389" lon="9.12914992310107" />
                <position distance="50" lat="42.2892028279603" lon="9.12895018234849" />
                <position distance="68" lat="42.2891914285719" lon="9.12873937748373" />
                <position distance="85" lat="42.2891838848591" lon="9.12852857261896" />
                <position distance="103" lat="42.2891778498888" lon="9.1283140797168" />
                <position distance="121" lat="42.289173072204" lon="9.12808743305504" />
                <position distance="141" lat="42.2891667857766" lon="9.12785307504237" />
                <position distance="160" lat="42.2891577333212" lon="9.12762274034321" />
                <position distance="177" lat="42.2891430649906" lon="9.12741227075458" />
                <position distance="194" lat="42.289112219587" lon="9.1272159665823" />
                <position distance="210" lat="42.2890667896718" lon="9.12703399546444" />
                <position distance="225" lat="42.2890075296164" lon="9.12685898132622" />
           ...
</positions>
Some segments (what we call courses):

<segments>
                <segment name="Total Trip" start="0" end="
</segments>
And finally some extra information:

<informations>
                <information distance="34" de="Georges de la Restonica\nL뿯½nge: 12,2 km - Gesamtanstieg: 790 hm" en="Georges de la Restonica\nLength: 12,2 km - Total ascent: 790 hm" />
                <information distance="103" de="Starth뿯½he: 489 m\nMax. H뿯½he: 1347 m" en="Initial height: 489 m\nMax. height: 1347 m" />
                <information distance="177" de="Durchschn. 7 % Steigung" en="Avg. gradient: 7 %" />
                <information distance="7629" de="1000 m 뿯½ber Meeresspiegel" en="1000 m above sea level" />
                <information distance="11199" de="1 km bis oben" en="1 km to top" />
                <information distance="12074" de="www.facebook.com/RealLifeVideo" en="www.facebook.com/RealLifeVideo" />
                <information distance="12144" de="www.RealLifeVideo.de" en="www.RealLifeVideo.de" /></informations>
Parsing this information shouldn't be too hard. The most work will be the conversion of the
RealLifeVideo class to a more general format so it can also handle input from these files.
 */
class VirtualTrainingFileParser : public QObject
{
    Q_OBJECT
public:
    explicit VirtualTrainingFileParser(const QList<QFileInfo>& videoFiles, QObject *parent = 0);

    /**
     * @brief parse a cycleops virtual training file.
     * @param inputFile the file to parse.
     * @return a RealLifeVideo. If parsing failed, the resulting RealLifeVideo will not be valid (check with isValid()).
     */
    RealLifeVideo parseVirtualTrainingFile(QFile &inputFile) const;
private:
    RealLifeVideo parseXml(QXmlStreamReader& reader) const;

    std::vector<ProfileEntry> convertProfileEntries(const std::vector<virtualtrainingfileparser::ProfileEntry> &virtualTrainingProfileEntries) const;
    std::vector<virtualtrainingfileparser::ProfileEntry> readProfileEntries(QXmlStreamReader &reader) const;
    QString findVideoFile(QString filename) const;
    std::vector<Course> readCourses(QXmlStreamReader& reader) const;
    std::vector<InformationBox> readInformationBoxes(QXmlStreamReader &reader) const;
    std::vector<GeoPosition> readPositions(QXmlStreamReader &reader) const;

    std::vector<virtualtrainingfileparser::DistanceMappingEntry> readDistanceMappings(QXmlStreamReader &reader) const;
    std::vector<DistanceMappingEntry> convertDistanceMappings(const std::vector<virtualtrainingfileparser::DistanceMappingEntry> & entries) const;
    const QList<QFileInfo> _videoFilesInfo;
};
}
#endif // VIRTUALTRAININGFILEPARSER_H
