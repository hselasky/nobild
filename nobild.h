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

#ifndef _NOBILD_H_
#define	_NOBILD_H_

#include <stdint.h>
#include <err.h>
#include <getopt.h>
#include <sysexits.h>
#include <unistd.h>
#include <pthread.h>
#include <iostream>

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
	TYPE_OTHER,
	TYPE_MAX,
};

enum {
	OWNER_FORTUM,
	OWNER_GRONNKONTAKT,
	OWNER_CLEVER,
	OWNER_EON,
	OWNER_OTHER,
	OWNER_MAX,
};

#endif					/* _NOBILD_H_ */
