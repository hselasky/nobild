/*-
 * Copyright (c) 2017-2023 Hans Petter Selasky
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
static QString output_file;

static QString icon_url[ICON_MAX] = {
	"http://www.selasky.org/charging/gfx_map_symbol_low.png",
	"http://www.selasky.org/charging/gfx_map_symbol_blue_low.png"
};

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
JavaScriptStringify(QString &output, const QString &variable, const QString &input)
{
	QByteArray ba(input.toUtf8());

	output += variable;
	output += ".push(";

	for (int x = 0; x != ba.count(); x++) {
		int ch = (uint8_t)ba[x];

		if (x != ba.count() - 1)
			output += QString("%1,").arg(ch);
		else
			output += QString("%1").arg(ch);
	}

	output += ");\n";
}

static void
NobildOutputGPXParts(nobild_head_t *phead, QString &output, const QString &variable)
{
	const nobild_cache *pc;
	int64_t owner_last = 0;
	int64_t kw_last = 0;
	int64_t type_last = 0;
	bool first = true;

	JavaScriptStringify(output, variable,
	    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
	    "<gpx xmlns=\"http://www.topografix.com/GPX/1/1\" "
	    "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
	    "xsi:schemaLocation=\"http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/1/1/gpx.xsd\" version=\"1.1\" "
	    "creator=\"Data provided by http://nobil.no and processed by http://www.selasky.org/charging\">\n");

	TAILQ_FOREACH(pc, phead, entry) {
		int64_t owner_mask = pc->get_owner_mask();
		int64_t kw_mask = pc->get_kw_mask();
		int64_t type_mask = pc->get_type_mask();

		if (owner_last != owner_mask || kw_last != kw_mask || type_last != type_mask) {
			owner_last = owner_mask;
			kw_last = kw_mask;
			type_last = type_mask;

			if (first == false)
				output += "}\n";
			first = false;
			output += QString("if ((owner_mask & %1) && (kw_mask & %2) && (type_mask & %3)) {\n")
			    .arg(owner_mask).arg(kw_mask).arg(type_mask);
		}
		JavaScriptStringify(output, variable, pc->output_gpx + QString("\n"));
	}
	if (first == false)
		output += "}\n";
	JavaScriptStringify(output, variable, "</gpx>\n");
}

static void
NobildOutputKMLIcon(QString &output, const QString &variable, const QString &icon_sel)
{
	for (int x = 0; x != ICON_MAX - 1; x++) {
		if (x != 0)
			output += "else ";
		output += QString("if (") + icon_sel + QString(" == %1)\n").arg(x);
		JavaScriptStringify(output, variable, icon_url[x]);
	}
	output += "else\n";
	JavaScriptStringify(output, variable, icon_url[ICON_MAX - 1]);
}

static void
NobildOutputKMLParts(nobild_head_t *phead, QString &output, const QString &variable, const QString &icon_sel)
{
	const nobild_cache *pc;
	int64_t owner_last = 0;
	int64_t kw_last = 0;
	int64_t type_last = 0;
	bool first = true;

	JavaScriptStringify(output, variable,
	    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
	    "<kml xmlns=\"http://www.opengis.net/kml/2.2\" xmlns:gx=\"http://www.google.com/kml/ext/2.2\">\n"
	    "<Document>\n"
	    "<name>EV charging stations</name>\n"
	    "<snippet>Data provided by http://nobil.no and processed by http://www.selasky.org/charging</snippet>\n"
	    "<LookAt>\n"
	    "<longitude>15</longitude>\n"
	    "<latitude>63</latitude>\n"
	    "<range>2560000</range>\n"
	    "</LookAt>\n"
	    "<Style id=\"waypoint_n\">\n"
	    "<IconStyle>\n"
	    "<Icon>\n"
	    "<href>");

	NobildOutputKMLIcon(output, variable, icon_sel);

	JavaScriptStringify(output, variable,
	    "</href>\n"
	    "</Icon>\n"
	    "</IconStyle>\n"
	    "</Style>\n"
	    "<Style id=\"waypoint_h\">\n"
	    "<IconStyle>\n"
	    "<scale>1.2</scale>\n"
	    "<Icon>\n"
	    "<href>");

	NobildOutputKMLIcon(output, variable, icon_sel);

	JavaScriptStringify(output, variable,
            "</href>\n"
	    "</Icon>\n"
	    "</IconStyle>\n"
	    "</Style>\n"
	    "<StyleMap id=\"waypoint\">\n"
	    "<Pair>\n"
	    "<key>normal</key>\n"
	    "<styleUrl>#waypoint_n</styleUrl>\n"
	    "</Pair>\n"
	    "<Pair>\n"
	    "<key>highlight</key>\n"
	    "<styleUrl>#waypoint_h</styleUrl>\n"
	    "</Pair>\n"
	    "</StyleMap>\n"
	    "<Folder>\n"
	    "<name>EV charging stations</name>\n");

	TAILQ_FOREACH(pc, phead, entry) {
		int64_t owner_mask = pc->get_owner_mask();
		int64_t kw_mask = pc->get_kw_mask();
		int64_t type_mask = pc->get_type_mask();

		if (owner_last != owner_mask || kw_last != kw_mask || type_last != type_mask) {
			owner_last = owner_mask;
			kw_last = kw_mask;
			type_last = type_mask;

			if (first == false)
				output += "}\n";
			first = false;
			output += QString("if ((owner_mask & %1) && (kw_mask & %2) && (type_mask & %3)) {\n")
			    .arg(owner_mask).arg(kw_mask).arg(type_mask);
		}
		JavaScriptStringify(output, variable, pc->output_kml + QString("\n"));
	}
	if (first == false)
		output += "}\n";

	JavaScriptStringify(output, variable,
	    "</Folder>\n"
	    "</Document>\n"
	    "</kml>\n");
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

					pc->output_gpx = QString("<wpt lat=\"%1\" lon=\"%2\"><name>%3</name></wpt>")
					    .arg(coord[0]).arg(coord[1]).arg(title);
					pc->output_kml = QString("<Placemark><name>%1</name><styleUrl>#waypoint</styleUrl><Point><coordinates>%2,%3</coordinates></Point></Placemark>")
					    .arg(title).arg(coord[1]).arg(coord[0]);
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

static int
NobildSortCompare(const void *pa, const void *pb)
{
	const nobild_cache *pc_a = *(const nobild_cache **)pa;
	const nobild_cache *pc_b = *(const nobild_cache **)pb;

	if (pc_a->get_owner_mask() > pc_b->get_owner_mask())
		return (1);
	else if (pc_a->get_owner_mask() < pc_b->get_owner_mask())
		return (-1);
	else if (pc_a->get_kw_mask() > pc_b->get_kw_mask())
		return (1);
	else if (pc_a->get_kw_mask() < pc_b->get_kw_mask())
		return (-1);
	else if (pc_a->get_type_mask() > pc_b->get_type_mask())
		return (1);
	else if (pc_a->get_type_mask() < pc_b->get_type_mask())
		return (-1);
	else
		return (0);
}

static void
NobildSortXML(nobild_head_t *phead)
{
	nobild_cache *pc;
	nobild_cache **ppc;
	size_t num;

	num = 0;
	TAILQ_FOREACH(pc, phead, entry)
		num++;

	if (num <= 1)
		return;

	ppc = new nobild_cache * [num];

	num = 0;
	TAILQ_FOREACH(pc, phead, entry)
		ppc[num++] = pc;

	mergesort(ppc, num, sizeof(ppc[0]), &NobildSortCompare);

	TAILQ_INIT(phead);

	for (size_t x = 0; x != num; x++)
		TAILQ_INSERT_TAIL(phead, ppc[x], entry);

	delete [] ppc;
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
	size_t kw_count[KW_MAX] = {};
	size_t owner_total = 0;
	nobild_cache *pc;
	QString js;

	TAILQ_FOREACH(pc, phead, entry) {
		owner_max[pc->owner]++;
		owner_total++;
		for (int x = 0; x != TYPE_MAX; x++)
			type_max[x] += pc->type[x];

		for (int x = 0; x != KW_MAX; x++)
			kw_count[x] += (pc->get_kw_mask() >> x) & 1;
	}

	js += "document.write(\'";
	js += "<form id=\"mainForm\" name=\"mainForm\">";

	js += QString("<h2>Make a selection among %1 EV charging stations</h2>").arg(owner_total);

	js += "<table style=\"width:100%\">";
	js += "<tr>";
	js += "<th><div align=\"left\"><b>Select power</b></div></th>";
	js += "<th><div align=\"left\"><b>Select vendor</b></div></th>";
	js += "<th><div align=\"left\"><b>Select plug</b></div></th>";
	js += "<th><div align=\"left\"><b>Select icon</b></div></th>";
	js += "</tr>";
	js += "<tr>";
	js += "<th>";
	js += "<div align=\"left\"><div align=\"top\">";

	for (int x = 0; x != KW_MAX; x++) {
		js += QString("<input type=\"checkbox\" name=\"kw_%1\" checked/> [%2 .. %3] kW (%4 stations)<br>")
		    .arg(x).arg((x == 0) ? 0 : (20 << (x - 1))).arg(20 << x).arg(kw_count[x]);
	}
	js += "</div></div>";
	js += "</th>";

	js += "<th>";
	js += "<div align=\"left\"><div align=\"top\">";

	for (int x = 0; x != OWNER_MAX; x++) {
		js += QString("<input type=\"checkbox\" name=\"owner_%1\" checked/> <a href=\"%2\">%3 (%4 stations)</a><br>")
		    .arg(x)
		    .arg(NobildOwner2Link(x))
		    .arg(NobildOwner2Str(x))
		    .arg(owner_max[x]);
	}
	js += "</div></div>";
	js += "</th>";

	js += "<th>";
	js += "<div align=\"left\"><div align=\"top\">";
	for (int x = 0; x != TYPE_MAX; x++) {
		js += QString("<input type=\"checkbox\" name=\"type_%1\" checked/> <a href=\"%2\">%3 (%4 plugs)</a><br>")
		    .arg(x)
		    .arg(NobildType2Link(x))
		    .arg(NobildType2StrFull(x))
		    .arg(type_max[x]);
	}
	js += "</div></div>";
	js += "</th>";
	js += "<th>";
	js += "<div align=\"left\"><div align=\"top\">";
	for (int x = 0; x != ICON_MAX; x++) {
		js += "<div align=\"left\">";
		js += QString("<input type=\"radio\" name=\"icon\" value=\"%1\"%2>").arg(x).arg((x == 0) ? " checked" : "");
		js += "</input><img src=\"";
		js += icon_url[x];
		js += "\"></img>";
		js += "</div><br>";
	}
	js += "</div></div>";
	js += "</th>";
	js += "</tr>";
	js += "</table><br>";
	js += "<button name=\"btn_gpx\">Download GPX</button> ";
	js += "<button name=\"btn_kml\">Download KML</button><br>";
	js += "</form>";
	js += "\');\n";

	js += "var kw_mask = 0;\n";
	js += "var owner_mask = 0;\n";
	js += "var type_mask = 0;\n";
	js += "var icon_sel = 0;\n";

	js += "function update_config() {\n";
	js += "kw_mask = 0;\n";
	js += "owner_mask = 0;\n";
	js += "type_mask = 0;\n";
	js += "icon_sel = document.mainForm.icon.value;\n";

	for (int x = 0; x != KW_MAX; x++)
		js += QString("if (document.mainForm.kw_%1.checked) kw_mask |= %2;\n").arg(x).arg(1 << x);
	for (int x = 0; x != OWNER_MAX; x++)
		js += QString("if (document.mainForm.owner_%1.checked) owner_mask |= %2;\n").arg(x).arg(1 << x);
	for (int x = 0; x != TYPE_MAX; x++)
		js += QString("if (document.mainForm.type_%1.checked) type_mask |= %2;\n").arg(x).arg(1 << x);
	js += "}\n";

	js += "document.mainForm.btn_gpx.onclick = function(){\n";
	js += "var gpx_string = [];\n";

	js += "update_config();\n";

	NobildOutputGPXParts(phead, js, QString("gpx_string"));

	js += "var gpx_len = gpx_string.length;\n";
	js += "var gpx_array = new Uint8Array(gpx_len);\n";
	js += "for (var x = 0; x != gpx_len; x++)\n";
	js += "	gpx_array[x] = gpx_string[x];\n";

	js += "var gpx_blob = new Blob([gpx_array], { type: \"application/x-gpx+xml\" });\n";
	js += "var a = document.createElement('a');\n";
	js += "a.href = window.URL.createObjectURL(gpx_blob);\n";
	js += "a.download = 'ev_charging_stations.gpx';\n";
	js += "a.click();\n";
	js += "}\n";

	js += "document.mainForm.btn_kml.onclick = function(){\n";
	js += "var kml_string = [];\n";

	js += "update_config();\n";

	NobildOutputKMLParts(phead, js, QString("kml_string"), QString("icon_sel"));

	js += "var kml_len = kml_string.length;\n";
	js += "var kml_array = new Uint8Array(kml_len);\n";
	js += "for (var x = 0; x != kml_len; x++)\n";
	js += "	kml_array[x] = kml_string[x];\n";

	js += "var kml_blob = new Blob([kml_array], { type: \"application/vnd.google-earth.kml+xml\" });\n";

	js += "var a = document.createElement('a');\n";
	js += "a.href = window.URL.createObjectURL(kml_blob);\n";
	js += "a.download = 'ev_charging_stations.kml';\n";
	js += "a.click();\n";
	js += "}\n";

	QFile file(output_file);

	if (!file.open(QFile::WriteOnly | QFile::Truncate))
		return (EINVAL);

	file.write(js.toUtf8());
	file.close();
	return (0);
}

static void
usage(void)
{
	fprintf(stderr, "usage: nobild -o <filename.js> -a <apikey>\n");
	exit(EX_USAGE);
}

static void *
worker(void *)
{
	nobild_head_t head;

	TAILQ_INIT(&head);

top:
	NobildCleanup(&head);

	QProcess fetch;

	QStringList args;

	args << "-qo" << "/dev/stdout" << QString("http://nobil.no/api/server/datadump.php?apikey=%1&format=xml&file=false").arg(apikey);

	fetch.start("fetch", args);
	fetch.waitForFinished(-1);

	if (fetch.exitStatus() != QProcess::NormalExit) {
		sleep(3600);
		goto top;
	}

	QByteArray data = fetch.readAllStandardOutput();

	NobildParseXML(data, &head);

	NobildSortXML(&head);

	if (NobildOutputJS(&head)) {
		sleep(3600);
		goto top;
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
			output_file = QString::fromLatin1(optarg);
			break;
		case 'a':
			apikey = QString::fromLatin1(optarg);
			break;
		default:
			usage();
			break;
		}
	}

	if (output_file.isEmpty())
		usage();

	if (apikey.isEmpty())
		usage();

	if (pthread_create(&td, 0, &worker, 0))
		err(EX_SOFTWARE, "Cannot create worker thread");

	return (app.exec());
}
