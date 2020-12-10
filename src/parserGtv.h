/* Copyright 2013-2020 Aven <turineaven@github>
 *
 * This program is free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see http://www.gnu.org/licenses/.
 */

#ifndef PARSERGTV_H
#define PARSERGTV_H

#include "parserBase.h"

class ParserGTV : public ParserBase
{
    Q_OBJECT
public:
    explicit ParserGTV(QObject *parent = 0) {}
    ~ParserGTV() {}

    inline static ParserGTV* instance() { return &s_instance; }
    static bool isSupported(const QUrl& url);

public slots:
    void replyFinished(void);

protected:
    void runParser(const QUrl &url);

private:
    static ParserGTV s_instance;
};

#endif // PARSERGTV_H
