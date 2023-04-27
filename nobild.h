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

#ifndef _NOBILD_H_
#define	_NOBILD_H_

#include <stdint.h>
#include <err.h>
#include <getopt.h>
#include <sysexits.h>
#include <unistd.h>
#include <pthread.h>
#include <iostream>
#include <ctype.h>
#include <stdlib.h>

#include <sys/queue.h>

#include <QApplication>
#include <QXmlStreamReader>
#include <QString>
#include <QProcess>
#include <QFile>
#include <QLocale>

#define	NOBILD_MAX_TAGS 32

enum {
	TYPE_CCS,
	TYPE_CHADEMO,
	TYPE_2,
	TYPE_TESLA,
	TYPE_OTHER,
	TYPE_MAX,
};

enum {
	OWNER_FORTUM,
	OWNER_GRONNKONTAKT,
	OWNER_BEE,
	OWNER_BKK,
	OWNER_CLEVER,
	OWNER_EON,
	OWNER_TESLA,
	OWNER_IONITY,
	OWNER_OTHER,
	OWNER_MAX,
};

enum {
	KW_0_20,
	KW_20_40,
	KW_40_80,
	KW_80_160,
	KW_160_MAX,
	KW_MAX,
};

enum {
	ICON_Z,
	ICON_S,
	ICON_MAX,
};

enum {
	KW_0_20_MASK = 1 << KW_0_20,
	KW_20_40_MASK = 1 << KW_20_40,
	KW_40_80_MASK = 1 << KW_40_80,
	KW_80_160_MASK = 1 << KW_80_160,
	KW_160_MAX_MASK = 1 << KW_160_MAX,
};

class nobild_cache {
public:
	TAILQ_CLASS_ENTRY(nobild_cache) entry;
	QString output_gpx;
	QString output_kml;
	int owner;
	float capacity_min;
	float capacity_max;
  	size_t type[TYPE_MAX];

	int64_t get_owner_mask() const {
		return (1LL << owner);
	}

	int64_t get_kw_mask() const {
		int64_t kw_mask = 0;

		if (capacity_max >= 0 && capacity_max < 20)
			kw_mask |= KW_0_20_MASK;
		if (capacity_max >= 20 && capacity_max < 40)
			kw_mask |= KW_20_40_MASK;
		if (capacity_max >= 40 && capacity_max < 80)
			kw_mask |= KW_40_80_MASK;
		if (capacity_max >= 80 && capacity_max < 160)
			kw_mask |= KW_80_160_MASK;
		if (capacity_max >= 160)
			kw_mask |= KW_160_MAX_MASK;
		return (kw_mask);
	}

	int64_t get_type_mask()  const {
		int64_t type_mask = 0;

		for (int x = 0; x != TYPE_MAX; x++) {
			if (type[x] != 0)
				type_mask |= 1LL << x;
		}
		return (type_mask);
	}
};

typedef TAILQ_CLASS_HEAD(, nobild_cache) nobild_head_t;

#endif					/* _NOBILD_H_ */
