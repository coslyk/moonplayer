/* Copyright 2013-2020 Yikun Liu <cos.lyk@gmail.com>
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

#ifndef PARSERYOUTUBEDL_H
#define PARSERYOUTUBEDL_H

#include "parserBase.h"
#include <QProcess>

class ParserYoutubeDL : public ParserBase
{
    Q_OBJECT
public:
    inline static ParserYoutubeDL* instance() { return &s_instance; }
    explicit ParserYoutubeDL(QObject *parent = 0);
    ~ParserYoutubeDL();

protected:
    void runParser(const QUrl &url);

private slots:
    void parseOutput(void);

private:
    QProcess m_process;
    
    static ParserYoutubeDL s_instance;
};

#endif // PARSERYOUTUBEDL_H
