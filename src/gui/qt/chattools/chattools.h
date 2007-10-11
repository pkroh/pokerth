/***************************************************************************
 *   Copyright (C) 2006 by FThauer FHammer   *
 *   f.thauer@web.de   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef CHATTOOLS_H
#define CHATTOOLS_H

#include <string>
#include <QtCore>
#include <QtGui>

class ChatTools : public QObject
{
Q_OBJECT

public:
	ChatTools(QLineEdit* l, QTreeWidget *t = NULL, QStringList *s = NULL);

	~ChatTools();

public slots:
	
	void fillChatLinesHistory(QString fillString);
	void showChatHistoryIndex(int index);
	int getChatLinesHistorySize() { return chatLinesHistory.size(); }
	
	void nickAutoCompletition();
	void setChatTextEdited();

private:
	QStringList chatLinesHistory;
	QString lastChatString;
	QStringList lastMatchStringList;
	int nickAutoCompletitionCounter;

	QLineEdit* myLineEdit;
	QTreeWidget *myNickTreeWidget;
	QStringList *myNickStringList;
// friend class GuiWrapper;
};

#endif