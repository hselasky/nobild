/*-
 * Copyright (c) 2017 Hans Petter Selasky. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "nobild.h"

static size_t owner_stats[OWNER_MAX];
static QString apikey;
static QString output_directory = ".";

static int
NobildStr2Owner(const QString & str)
{
	QString upper = str.toUpper();

	if (upper.indexOf("CLEVER") > -1)
		return (OWNER_CLEVER);
	else if (upper.indexOf("E.ON") > -1)
		return (OWNER_EON);
	else if (upper.indexOf("FORTUM") > -1)
		return (OWNER_FORTUM);
	else if (upper.indexOf("GRØNN KONTAKT") > -1)
		return (OWNER_GRONNKONTAKT);
	else
		return (OWNER_OTHER);
}

static	QString
NobildOwner2Str(int value)
{

	switch (value) {
	case OWNER_CLEVER:
		return ("Clever");
	case OWNER_EON:
		return ("E.ON");
	case OWNER_FORTUM:
		return ("Fortum");
	case OWNER_GRONNKONTAKT:
		return ("Grønn Kontakt");
	default:
		return ("Unknown");
	}
}

static	QString
NobildType2Str(int value)
{

	switch (value) {
	case TYPE_CCS:
		return ("CCS");
	case TYPE_CHADEMO:
		return ("CHAdeMO");
	default:
		return ("Unknown");
	}
}

static void
NobildParseXML(QString & output, const QByteArray & data, float kw_min)
{
	QXmlStreamReader:: TokenType token = QXmlStreamReader::NoToken;
	QXmlStreamReader xml(data);
	QString tags[NOBILD_MAX_TAGS];
	QString position;
	QString name;
	QString owned_by;
	QString attrtypeid;
	QString attrvalid;
	QString trans;
	float opt_capacity_min;
	float opt_capacity_max;
	size_t opt_type[TYPE_MAX];
	int opt_public;
	int opt_24h;
	int opt_owner;
	size_t si = 0;

	output =
	    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
	    "<gpx xmlns=\"http://www.topografix.com/GPX/1/1\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/1/1/gpx.xsd\" version=\"1.1\" creator=\"home.selasky.org/charging - datagrunnlaget er hentet fra http://nobil.no\">";

	while (!xml.atEnd()) {
		if (token == QXmlStreamReader::NoToken)
			token = xml.readNext();

		switch (token) {
		case QXmlStreamReader:: Invalid:
			goto done;
		case QXmlStreamReader:: StartElement:
			if (si < NOBILD_MAX_TAGS)
				tags[si] = xml.name().toString().toLower();
			si++;

			if (si == 2 &&
			    tags[0] == "chargerstations" &&
			    tags[1] == "chargerstation") {
				position = QString();
				name = QString();
				owned_by = QString();
				memset(opt_type, 0, sizeof(opt_type));
				opt_24h = 0;
				opt_public = 0;
				opt_capacity_min = 0;
				opt_capacity_max = 0;
				opt_owner = OWNER_OTHER;
			} else if (si == 4 &&
				   tags[0] == "chargerstations" &&
				   tags[1] == "chargerstation" &&
				   tags[2] == "metadata" &&
				   tags[3] == "position") {
				token = xml.readNext();
				if (token != QXmlStreamReader::Characters)
					continue;
				position = xml.text().toString();
			} else if (si == 4 &&
				   tags[0] == "chargerstations" &&
				   tags[1] == "chargerstation" &&
				   tags[2] == "metadata" &&
				   tags[3] == "name") {
				token = xml.readNext();
				if (token != QXmlStreamReader::Characters)
					continue;
				name = xml.text().toString();
			} else if (si == 4 &&
				   tags[0] == "chargerstations" &&
				   tags[1] == "chargerstation" &&
				   tags[2] == "metadata" &&
				   tags[3] == "owned_by") {
				token = xml.readNext();
				if (token != QXmlStreamReader::Characters)
					continue;
				owned_by = xml.text().toString();
			} else if (si == 5 &&
				   tags[0] == "chargerstations" &&
				   tags[1] == "chargerstation" &&
				   tags[2] == "attributes" &&
				   tags[3] == "station" &&
				   tags[4] == "attribute") {
				attrtypeid = QString();
				attrvalid = QString();
				trans = QString();
			} else if (si == 6 &&
				   tags[0] == "chargerstations" &&
				   tags[1] == "chargerstation" &&
				   tags[2] == "attributes" &&
				   tags[3] == "station" &&
				   tags[4] == "attribute" &&
				   tags[5] == "attrtypeid") {
				token = xml.readNext();
				if (token != QXmlStreamReader::Characters)
					continue;
				attrtypeid = xml.text().toString().trimmed();
			} else if (si == 6 &&
				   tags[0] == "chargerstations" &&
				   tags[1] == "chargerstation" &&
				   tags[2] == "attributes" &&
				   tags[3] == "station" &&
				   tags[4] == "attribute" &&
				   tags[5] == "attrvalid") {
				token = xml.readNext();
				if (token != QXmlStreamReader::Characters)
					continue;
				attrvalid = xml.text().toString().trimmed();
			} else if (si == 6 &&
				   tags[0] == "chargerstations" &&
				   tags[1] == "chargerstation" &&
				   tags[2] == "attributes" &&
				   tags[3] == "station" &&
				   tags[4] == "attribute" &&
				   tags[5] == "trans") {
				token = xml.readNext();
				if (token != QXmlStreamReader::Characters)
					continue;
				trans = xml.text().toString().trimmed();
			} else if (si == 6 &&
				   tags[0] == "chargerstations" &&
				   tags[1] == "chargerstation" &&
				   tags[2] == "attributes" &&
				   tags[3] == "connectors" &&
				   tags[4] == "connector" &&
				   tags[5] == "attribute") {
				attrtypeid = QString();
				attrvalid = QString();
				trans = QString();
			} else if (si == 7 &&
				   tags[0] == "chargerstations" &&
				   tags[1] == "chargerstation" &&
				   tags[2] == "attributes" &&
				   tags[3] == "connectors" &&
				   tags[4] == "connector" &&
				   tags[5] == "attribute" &&
				   tags[6] == "attrtypeid") {
				token = xml.readNext();
				if (token != QXmlStreamReader::Characters)
					continue;
				attrtypeid = xml.text().toString().trimmed();
			} else if (si == 7 &&
				   tags[0] == "chargerstations" &&
				   tags[1] == "chargerstation" &&
				   tags[2] == "attributes" &&
				   tags[3] == "connectors" &&
				   tags[4] == "connector" &&
				   tags[5] == "attribute" &&
				   tags[6] == "attrvalid") {
				token = xml.readNext();
				if (token != QXmlStreamReader::Characters)
					continue;
				attrvalid = xml.text().toString().trimmed();
			} else if (si == 7 &&
				   tags[0] == "chargerstations" &&
				   tags[1] == "chargerstation" &&
				   tags[2] == "attributes" &&
				   tags[3] == "connectors" &&
				   tags[4] == "connector" &&
				   tags[5] == "attribute" &&
				   tags[6] == "trans") {
				token = xml.readNext();
				if (token != QXmlStreamReader::Characters)
					continue;
				trans = xml.text().toString().trimmed();
			}
			break;

		case QXmlStreamReader:: EndElement:
			if (si == 0)
				break;

			if (si == 2 &&
			    tags[0] == "chargerstations" &&
			    tags[1] == "chargerstation") {
				QString title;
				float coord[2] = {};
				float factor = 1.0;
				int owner;
				int x;
				int offset;

				owner = NobildStr2Owner(owned_by);
				if (owner == OWNER_OTHER)
					owner = NobildStr2Owner(name);

				for (x = 2, offset = position.size(); x > -1 && offset--;) {
					if (position[offset].isNumber()) {
						if (x >= 0 && x < 2) {
							coord[x] += position[offset].digitValue() * factor;
							factor *= 10.0;
						}
					} else if (position[offset] == '.') {
						if (x >= 0 && x < 2) {
							coord[x] /= factor;
							factor = 1.0;
						}
					} else if (position[offset] == '(' || position[offset] == ')' || position[offset] == ',') {
						x--;
						factor = 1.0;
					} else {
						break;
					}
				}

				if (offset == 0 && opt_public && opt_24h && x == -1 && opt_capacity_max >= kw_min) {
					title += NobildOwner2Str(owner);
					if (opt_capacity_max != 0.0) {
						if (opt_capacity_min == opt_capacity_max) {
							title += QString(" %1kW").arg((int)opt_capacity_min);
						} else {
							title += QString(" %1-%2kW")
							  .arg((int)opt_capacity_min).arg((int)opt_capacity_max);
						}
					}
					for (x = 0; x != TYPE_MAX; x++) {
						if (opt_type[x] == 0)
							continue;
						title += QString(" %1:%2").arg(NobildType2Str(x)).arg(opt_type[x]);
					}
					output += QString("<wpt lat=\"%1\" lon=\"%2\"><name>%3</name></wpt>").arg(coord[0]).arg(coord[1]).arg(title);
					owner_stats[owner]++;
				}
			} else if (si == 5 &&
				   tags[0] == "chargerstations" &&
				   tags[1] == "chargerstation" &&
				   tags[2] == "attributes" &&
				   tags[3] == "station" &&
				   tags[4] == "attribute") {

				if (attrtypeid == "24" && attrvalid == "1")
					opt_24h = 1;
				else if (attrtypeid == "2" && attrvalid == "1")
					opt_public = 1;
			} else if (si == 6 &&
				   tags[0] == "chargerstations" &&
				   tags[1] == "chargerstation" &&
				   tags[2] == "attributes" &&
				   tags[3] == "connectors" &&
				   tags[4] == "connector" &&
				   tags[5] == "attribute") {

				if (attrtypeid == "5") {
					int offset = trans.indexOf("kW");
					float factor = 1.0;
					float capacity = 0.0;

					for (; offset--; ) {
						if (trans[offset] == ' ') {
							if (capacity != 0.0)
								break;
						} else if (trans[offset] == ',') {
							capacity /= factor;
							factor = 1.0;
						} else if (trans[offset].isNumber()) {
							capacity += trans[offset].digitValue() * factor;
							factor *= 10.0;
						} else {
							break;
						}
					}

					if (capacity == 0.0)
						;
					else if (opt_capacity_min == 0.0)
						opt_capacity_min = opt_capacity_max = capacity;
					else if (capacity < opt_capacity_min)
						opt_capacity_min = capacity;
					else if (capacity > opt_capacity_max)
						opt_capacity_max = capacity;
				} else if (attrtypeid == "4") {
					if (trans.indexOf("CCS") > -1)
						opt_type[TYPE_CCS]++;
					else if (trans.indexOf("CHAdeMO") > -1)
						opt_type[TYPE_CHADEMO]++;
					else
						opt_type[TYPE_OTHER]++;
				}
			}
			si--;
			if (si < NOBILD_MAX_TAGS)
				tags[si] = QString();
			break;
		default:
			break;
		}
		token = QXmlStreamReader::NoToken;
	}
done:
	output += "</gpx>";

	int x;

	output += "<!-- ";
	for (x = 0; x != OWNER_MAX; x++) {
		output += QString("%1:%2 ").arg(NobildOwner2Str(x)).arg(owner_stats[x]);
	}
	output += "-->";
}

static void
usage(void)
{
	fprintf(stderr, "usage: nobild\n");
	exit(EX_USAGE);
}

static void *
worker(void *arg)
{
top:;
	QProcess fetch;

	fetch.start(QString("fetch -qo /dev/stdout http://nobil.no/api/server/datadump.php?apikey=%1&format=xml&file=false").arg(apikey));
	fetch.waitForFinished(-1);

	if (fetch.exitStatus() != QProcess::NormalExit) {
		sleep(3600);
		goto top;
	}

	QByteArray data = fetch.readAllStandardOutput();		  
	QProcess convert;

	for (int x = 0; ; x++) {
		QString suffix;
		float limit;

		if (x == 0) {
			suffix = "all";
			limit = 0;
		} else if (x == 1) {
			suffix = "20";
			limit = 20;
		} else if (x == 2) {
			suffix = "40";
			limit = 40;
		} else {
			break;
		}
		    
		QString output;

		NobildParseXML(output, data, limit);

		QFile file(output_directory + "/ev_charger_stations_" + suffix + ".gpx");

		if (!file.open(QFile::WriteOnly | QFile::Truncate)) {
			sleep(3600);
			goto top;
		}
		file.write(output.toUtf8());
		file.close();

		convert.start(QString("gpsbabel -i gpx -o kml ") + output_directory + "/ev_charger_stations_" + suffix + ".gpx " +
			      output_directory + "/ev_charger_stations_" + suffix + ".kml");
		convert.waitForFinished(-1);
				      
		if (convert.exitStatus() != QProcess::NormalExit) {
			sleep(3600);
			goto top;
		}
	}
	exit(0);
	return (NULL);
}

int
main(int argc, char **argv)
{
	QApplication app(argc, argv);
	const char *optstring = "a:o:h?";
	pthread_t td;
	int c;

	while ((c = getopt(argc, argv, optstring)) != -1) {
		switch (c) {
		case 'o':
			output_directory = QString::fromLatin1(optarg);
			break;
		case 'a':
			apikey = QString::fromLatin1(optarg);
			break;
		default:
			usage();
			break;
		}
	}

	if (apikey.isEmpty())
		usage();

	if (pthread_create(&td, 0, &worker, 0))
		err(EX_SOFTWARE, "Cannot create worker thread");

	return (app.exec());
}
