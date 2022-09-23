/*-
 * Copyright (c) 2017-2022 Hans Petter Selasky. All rights reserved.
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

static QString apikey;
static QString output_directory = ".";

static int
NobildStr2Owner(const QString & str)
{
	QString upper = str.toUpper();

	if (upper.indexOf("BEE") == 0)
		return (OWNER_BEE);
	else if (upper == "EVINY" || upper == "BKK")
		return (OWNER_BKK);
	else if (upper.indexOf("CLEVER") > -1)
		return (OWNER_CLEVER);
	else if (upper.indexOf("E.ON") > -1)
		return (OWNER_EON);
	else if (upper.indexOf("FORTUM") > -1 || upper == "RECHARGE")
		return (OWNER_FORTUM);
	else if (upper.indexOf("GRØNN KONTAKT") > -1 || upper == "MER")
		return (OWNER_GRONNKONTAKT);
	else if (upper.indexOf("TESLA") > -1)
		return (OWNER_TESLA);
	else if (upper.indexOf("IONITY") > -1)
		return (OWNER_IONITY);
	else
		return (OWNER_OTHER);
}

static	QString
NobildOwner2Str(int value)
{

	switch (value) {
	case OWNER_BEE:
		return ("Bee");
	case OWNER_BKK:
		return ("Eviny");
	case OWNER_CLEVER:
		return ("Clever");
	case OWNER_EON:
		return ("E.ON");
	case OWNER_FORTUM:
		return ("Recharge");
	case OWNER_GRONNKONTAKT:
		return ("Mer");
	case OWNER_TESLA:
		return ("Tesla");
	case OWNER_IONITY:
		return ("Ionity");
	default:
		return ("Other");
	}
}

static	QString
NobildOwner2Link(int value)
{

	switch (value) {
	case OWNER_BEE:
		return ("https://bee.se");
	case OWNER_BKK:
		return ("https://www.eviny.no");
	case OWNER_CLEVER:
		return ("https://clever.dk");
	case OWNER_EON:
		return ("https://www.eon.com");
	case OWNER_FORTUM:
		return ("http://www.rechargeinfra.com");
	case OWNER_GRONNKONTAKT:
		return ("https://no.mer.eco");
	case OWNER_TESLA:
		return ("https://www.tesla.com");
	case OWNER_IONITY:
		return ("https://ionity.eu");
	default:
		return ("index.html");
	}
}

static	QString
NobildType2Str(int value)
{

	switch (value) {
	case TYPE_CCS:
		return ("CCS");
	case TYPE_CHADEMO:
		return ("CHA");
	case TYPE_2:
		return ("TP2");
	case TYPE_TESLA:
		return ("TES");
	default:
		return ("UNK");
	}
}

static	QString
NobildType2StrFull(int value)
{

	switch (value) {
	case TYPE_CCS:
		return ("CCS EUR");
	case TYPE_CHADEMO:
		return ("CHAdeMO");
	case TYPE_2:
		return ("Type 2");
	case TYPE_TESLA:
		return ("Tesla connector");
	default:
		return ("Other");
	}
}

static	QString
NobildType2Link(int value)
{

	switch (value) {
	case TYPE_CCS:
		return ("https://en.wikipedia.org/wiki/Combined_Charging_System");
	case TYPE_CHADEMO:
		return ("https://en.wikipedia.org/wiki/CHAdeMO");
	case TYPE_2:
		return ("https://en.wikipedia.org/wiki/Type_2_connector");
	case TYPE_TESLA:
		return ("https://en.wikipedia.org/wiki/Tesla_Supercharger");
	default:
		return ("index.html");
	}
}

static void
NobildOutputXML(nobild_head_t *phead, QString & output, uint64_t type_mask, uint64_t kw_mask, uint64_t owner_mask)
{
	size_t owner_stats[OWNER_MAX] = {};
	nobild_cache *pc;

	output =
	    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
	    "<gpx xmlns=\"http://www.topografix.com/GPX/1/1\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/1/1/gpx.xsd\" version=\"1.1\" creator=\"www.selasky.org/charging - datagrunnlaget er hentet fra http://nobil.no\">\n";

	TAILQ_FOREACH(pc, phead, entry) {
		bool found = 0;
		int x;

		if (!(owner_mask & (1 << pc->owner)))
			continue;

		if (kw_mask & KW_0_20_MASK)
			found |= (pc->capacity_max >= 0 && pc->capacity_max < 20);
		if (kw_mask & KW_20_40_MASK)
			found |= (pc->capacity_max >= 20 && pc->capacity_max < 40);
		if (kw_mask & KW_40_80_MASK)
			found |= (pc->capacity_max >= 40 && pc->capacity_max < 80);
		if (kw_mask & KW_80_160_MASK)
			found |= (pc->capacity_max >= 80 && pc->capacity_max < 160);
		if (kw_mask & KW_160_MAX_MASK)
			found |= (pc->capacity_max >= 160);

		if (found == 0)
			continue;

		found = 0;
		for (x = 0; x != TYPE_MAX; x++)
			found |= (pc->type[x] != 0 && (type_mask & (1 << x)));

		if (found == 0)
			continue;

		output += pc->output;
		output += "\n";

		owner_stats[pc->owner]++;
	}
	output += "</gpx>\n";

	output += "<!-- ";
	for (int x = 0; x != OWNER_MAX; x++) {
		output += QString("%1:%2 ").arg(NobildOwner2Str(x)).arg(owner_stats[x]);
	}
	output += "-->";
}

static void
NobildParseXML(const QByteArray & data, nobild_head_t *phead)
{
	QXmlStreamReader:: TokenType token = QXmlStreamReader::NoToken;
	QXmlStreamReader xml(data);
	QString tags[NOBILD_MAX_TAGS];
	QString position;
	QString name;
	QString owned_by;
	QString user_comment;
	QString attrtypeid;
	QString attrvalid;
	QString trans;
	float opt_capacity_min;
	float opt_capacity_max;
	size_t opt_type[TYPE_MAX];
	int opt_public;
	int opt_24h;
	size_t si = 0;

	while (!xml.atEnd()) {
		if (token == QXmlStreamReader::NoToken)
			token = xml.readNext();

		switch (token) {
		case QXmlStreamReader:: Invalid:
			return;
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
				user_comment = QString();
				memset(opt_type, 0, sizeof(opt_type));
				opt_24h = 0;
				opt_public = 0;
				opt_capacity_min = 0;
				opt_capacity_max = 0;
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
			} else if (si == 4 &&
				   tags[0] == "chargerstations" &&
				   tags[1] == "chargerstation" &&
				   tags[2] == "metadata" &&
				   tags[3] == "user_comment") {
				token = xml.readNext();
				if (token != QXmlStreamReader::Characters)
					continue;
				user_comment = xml.text().toString();
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
				if (owner == OWNER_OTHER) {
					owner = NobildStr2Owner(name);
					if (owner == OWNER_OTHER)
						owner = NobildStr2Owner(user_comment);
				}

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

				if (offset == 0 && opt_public && x == -1) {
					if (owner == OWNER_OTHER && !name.isEmpty()) {
						int strip = name.indexOf(',');
						if (strip > -1)
							title += name.left(strip).trimmed();
						else
							title += name;
					} else if (!name.isEmpty()) {
						QString tt;

						int strip = name.indexOf(',');
						if (strip > -1)
							tt += name.left(strip).trimmed();
						else
							tt += name.trimmed();

						if (NobildStr2Owner(tt) == OWNER_OTHER) {
							title += NobildOwner2Str(owner);
							title += " ";
						}
						title += tt;
					} else {
						title += NobildOwner2Str(owner);
					}

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
					if (!opt_24h)
						title += " not open 24/7";

					nobild_cache *pc = new nobild_cache;

					pc->output = QString("<wpt lat=\"%1\" lon=\"%2\"><name>%3</name></wpt>")
					    .arg(coord[0]).arg(coord[1]).arg(title);
					pc->owner = owner;
					pc->capacity_min = opt_capacity_min;
					pc->capacity_max = opt_capacity_max;
					for (int z = 0; z != TYPE_MAX; z++)
						pc->type[z] = opt_type[z];
					TAILQ_INSERT_TAIL(phead, pc, entry);
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
					else if (trans.indexOf("Type 2") > -1)
						opt_type[TYPE_2]++;
					else if (trans.indexOf("Tesla Connector Model") > -1)
						opt_type[TYPE_TESLA]++;
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
}

static void
NobildCleanup(nobild_head_t *phead)
{
	nobild_cache *pc;

	while ((pc = TAILQ_FIRST(phead))) {
		TAILQ_REMOVE(phead, pc, entry);
		delete pc;
	}
}

static int
NobildOutputJS(nobild_head_t *phead)
{
	size_t type_max[TYPE_MAX] = {};
	size_t owner_max[OWNER_MAX] = {};
	size_t kw_more[KW_MAX] = {};
	size_t kw_less[KW_MAX] = {};
	size_t owner_total = 0;
	nobild_cache *pc;
	QString js;

	TAILQ_FOREACH(pc, phead, entry) {
		owner_max[pc->owner]++;
		owner_total++;
		for (int x = 0; x != TYPE_MAX; x++)
			type_max[x] += pc->type[x];

		for (int x = 0; x != KW_MAX; x++) {
			kw_more[x] += (pc->capacity_max >= (20.0 * (1 << x)));
			kw_less[x] += (pc->capacity_max < (20.0 * (1 << x)));
		}
	}

	js += "document.write(\'";
	js += "<form id=\"mainForm\" name=\"mainForm\">";

	js += "<table style=\"width:100%\">";
	js += "<tr>";
	js += "<th>";
	js += "<div align=\"left\"><div align=\"top\">";
	js += "<input type=\"radio\" name=\"kw\" value=\"-1\" checked=\"checked\" /> All kW rates<br>";
	js += QString("<input type=\"radio\" name=\"kw\" value=\"1\" /> Less than 20 kW (%1 stations)<br>").arg(kw_less[KW_0_20]);
	js += QString("<input type=\"radio\" name=\"kw\" value=\"3\" /> Less than 40 kW (%1 stations)<br>").arg(kw_less[KW_20_40]);
	js += QString("<input type=\"radio\" name=\"kw\" value=\"7\" /> Less than 80 kW (%1 stations)<br>").arg(kw_less[KW_40_80]);
	js += QString("<input type=\"radio\" name=\"kw\" value=\"15\" /> Less than 160 kW (%1 stations)<br>").arg(kw_less[KW_80_160]);
	js += QString("<input type=\"radio\" name=\"kw\" value=\"-2\" /> More than 20 kW (%1 stations)<br>").arg(kw_more[KW_0_20]);
	js += QString("<input type=\"radio\" name=\"kw\" value=\"-4\" /> More than 40 kW (%1 stations)<br>").arg(kw_more[KW_20_40]);
	js += QString("<input type=\"radio\" name=\"kw\" value=\"-8\" /> More than 80 kW (%1 stations)<br>").arg(kw_more[KW_40_80]);
	js += QString("<input type=\"radio\" name=\"kw\" value=\"-16\" /> More than 160 kW (%1 stations)<br>").arg(kw_more[KW_80_160]);
	js += "</div></div>";
	js += "</th>";

	js += "<th>";
	js += "<div align=\"left\"><div align=\"top\">";
	js += QString("<input type=\"radio\" name=\"station\" value=\"-1\" checked=\"checked\" /> All stations (%1 in total)<br>").arg(owner_total);

	for (int x = 0; x != OWNER_MAX; x++) {
		js += QString("<input type=\"radio\" name=\"station\" value=\"%1\" /> <a href=\"%2\">%3 (%4)</a><br>")
		    .arg(1ULL << x)
		    .arg(NobildOwner2Link(x))
		    .arg(NobildOwner2Str(x))
		    .arg(owner_max[x]);
	}
	js += "</div></div>";
	js += "</th>";

	js += "<th>";
	js += "<div align=\"left\"><div align=\"top\">";
	js += "<input type=\"radio\" name=\"connector\" value=\"-1\" checked=\"checked\" /> All connectors<br>";
	for (int x = 0; x != TYPE_MAX; x++) {
		js += QString("<input type=\"radio\" name=\"connector\" value=\"%1\" /> <a href=\"%2\">%3 (%4)</a><br>")
		    .arg(1ULL << x)
		    .arg(NobildType2Link(x))
		    .arg(NobildType2StrFull(x))
		    .arg(type_max[x]);
	}
	js += "</div></div>";
	js += "</th>";
	js += "</table><br>";
	js += "<button name=\"btn_gpx\">Download GPX</button> ";
	js += "<button name=\"btn_kml\">Download KML</button><br>";
	js += "</form>";
	js += "\');\n";

	js += "document.mainForm.btn_gpx.onclick = function(){\n";
	js += "var fname = \"ev_charger_stations_\" + document.mainForm.connector.value + "
	    "\"_\" + document.mainForm.kw.value + \"_\" + document.mainForm.station.value + \".gpx\";";
	js += "window.open(fname);\n";
	js += "}\n";

	js += "document.mainForm.btn_kml.onclick = function(){\n";
	js += "var fname = \"ev_charger_stations_\" + document.mainForm.connector.value + "
	    "\"_\" + document.mainForm.kw.value + \"_\" + document.mainForm.station.value + \".kml\";";
	js += "window.open(fname);\n";
	js += "}\n";

	QFile file(output_directory + "/ev_charger_stations.js");

	if (!file.open(QFile::WriteOnly | QFile::Truncate))
		return (EINVAL);

	file.write(js.toUtf8());
	file.close();
	return (0);
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
	int64_t owner_mask;
	int64_t type_mask;
	int64_t kw_mask;
	QString suffix;
	nobild_head_t head;

	TAILQ_INIT(&head);

top:
	NobildCleanup(&head);

	QProcess fetch;

	fetch.start(QString("fetch -qo /dev/stdout http://nobil.no/api/server/datadump.php?apikey=%1&format=xml&file=false").arg(apikey));
	fetch.waitForFinished(-1);

	if (fetch.exitStatus() != QProcess::NormalExit) {
		sleep(3600);
		goto top;
	}

	QByteArray data = fetch.readAllStandardOutput();
	QProcess convert;

	NobildParseXML(data, &head);

	if (NobildOutputJS(&head)) {
		sleep(3600);
		goto top;
	}

	kw_mask = (1ULL << KW_MAX) - 1;
	while (kw_mask != -1LL) {
		kw_mask >>= 1;
		if (kw_mask == 0)
			kw_mask = -(1ULL << (KW_MAX - 1));

		type_mask = (1ULL << TYPE_MAX);
		while (type_mask != -1LL) {
			type_mask >>= 1;
			if (type_mask == 0)
				type_mask = -1LL;
		  
			owner_mask = (1ULL << OWNER_MAX);
			while (owner_mask != -1LL) {
				owner_mask >>= 1;
				if (owner_mask == 0)
					owner_mask = -1LL;

				suffix = QString("%1_%2_%3")
				  .arg((int64_t)type_mask)
				  .arg((int64_t)kw_mask)
				  .arg((int64_t)owner_mask);

				QString output;

				NobildOutputXML(&head, output, type_mask, kw_mask, owner_mask);

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
		}
	}

	NobildCleanup(&head);

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
