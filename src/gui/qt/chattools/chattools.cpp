/*****************************************************************************
 * PokerTH - The open source texas holdem engine                             *
 * Copyright (C) 2006-2011 Felix Hammer, Florian Thauer, Lothar May          *
 *                                                                           *
 * This program is free software: you can redistribute it and/or modify      *
 * it under the terms of the GNU Affero General Public License as            *
 * published by the Free Software Foundation, either version 3 of the        *
 * License, or (at your option) any later version.                           *
 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU Affero General Public License for more details.                       *
 *                                                                           *
 * You should have received a copy of the GNU Affero General Public License  *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.     *
 *****************************************************************************/
#include "chattools.h"
#include "session.h"
#include "configfile.h"
#include "gametablestylereader.h"
#include "gamelobbydialogimpl.h"
#include <iostream>


using namespace std;


ChatTools::ChatTools(QLineEdit* l, ConfigFile *c, ChatType ct, QTextBrowser *b, QStandardItemModel *m, gameLobbyDialogImpl *lo) : nickAutoCompletitionCounter(0), myLineEdit(l), myNickListModel(m), myNickStringList(NULL), myTextBrowser(b), myChatType(ct), myConfig(c), myNick(""), myLobby(lo)
{
	myNick = QString::fromUtf8(myConfig->readConfigString("MyName").c_str());
	ignoreList = myConfig->readConfigStringList("PlayerIgnoreList");

}

ChatTools::~ChatTools()
{
}

void ChatTools::sendMessage()
{

	if(myLineEdit->text().size() && mySession) {
		fillChatLinesHistory(myLineEdit->text());
		QString chatText(myLineEdit->text());
		if(myChatType == INGAME_CHAT) {
			mySession->sendGameChatMessage(chatText.toUtf8().constData());
		} else {
			// Parse user name for private messages.
			if(chatText.indexOf(QString("/msg ")) == 0) {
				chatText.remove(0, 5);
				unsigned playerId = parsePrivateMessageTarget(chatText);
				if (playerId)
					mySession->sendPrivateChatMessage(playerId, chatText.toUtf8().constData());
			} else {
				mySession->sendLobbyChatMessage(chatText.toUtf8().constData());
			}
		}
		myLineEdit->setText("");
	}
}

void ChatTools::receiveMessage(QString playerName, QString message, bool pm)
{

	if(myTextBrowser) {

		message = message.replace("<","&lt;");
		message = message.replace(">","&gt;");

		//refresh myNick if it was changed during runtime
		myNick = QString::fromUtf8(myConfig->readConfigString("MyName").c_str());

		QString tempMsg;

		if(myChatType == INET_LOBBY_CHAT && playerName == "(chat bot)" && message.startsWith(myNick)) {

			tempMsg = QString("<span style=\"font-weight:bold; color:red;\">"+message+"</span>");
			//play beep sound only in INET-lobby-chat
			if(myLobby->isVisible() && myConfig->readConfigInt("PlayLobbyChatNotification")) {
				myLobby->getMyW()->getMySDLPlayer()->playSound("lobbychatnotify",0);
			}
		} else if(message.contains(myNick, Qt::CaseInsensitive)) {

			switch (myChatType) {
			case INET_LOBBY_CHAT: {
				tempMsg = QString("<span style=\"font-weight:bold; color:"+myLobby->palette().link().color().name()+";\">"+message+"</span>");
				//play beep sound only in INET-lobby-chat
				//						TODO dont play when message is from yourself
				if(myLobby->isVisible() && myConfig->readConfigInt("PlayLobbyChatNotification")) {
					myLobby->getMyW()->getMySDLPlayer()->playSound("lobbychatnotify",0);
				}
			}
			break;
			case LAN_LOBBY_CHAT:
				tempMsg = QString("<span style=\"font-weight:bold;\">"+message+"</span>");
				break;
			case INGAME_CHAT:
				tempMsg = QString("<span style=\"color:#"+myStyle->getChatTextNickNotifyColor()+";\">"+message+"</span>");
				break;
			default:
				tempMsg = message;
			}
		} else if(playerName == myNick) {
			switch (myChatType) {
			case INET_LOBBY_CHAT:
				tempMsg = QString("<span style=\"font-weight:normal; color:"+myLobby->palette().link().color().name()+";\">"+message+"</span>");
				break;
			case LAN_LOBBY_CHAT:
				tempMsg = QString("<span style=\"font-weight:normal;\">"+message+"</span>");
				break;
			case INGAME_CHAT:
				tempMsg = QString("<span style=\"color:#"+myStyle->getChatLogTextColor()+";\">"+message+"</span>");
				break;
			default:
				tempMsg = message;
			}
		} else {
			switch (myChatType) {
			case INET_LOBBY_CHAT:
				tempMsg = QString("<span style=\"font-weight:normal; color:"+myLobby->palette().text().color().name()+";\">"+message+"</span>");
				break;
			case LAN_LOBBY_CHAT:
				tempMsg = QString("<span style=\"font-weight:normal;\">"+message+"</span>");
				break;
			case INGAME_CHAT:
				tempMsg = QString("<span style=\"color:#"+myStyle->getChatLogTextColor()+";\">"+message+"</span>");
				break;
			default:
				tempMsg = message;
			}

		}

		bool nickFoundOnIgnoreList = false;
		list<std::string>::iterator it1;
		for(it1=ignoreList.begin(); it1 != ignoreList.end(); ++it1) {

			if(playerName == QString::fromUtf8(it1->c_str())) {
				nickFoundOnIgnoreList = true;
			}
		}

		if(!nickFoundOnIgnoreList) {

			if(message.indexOf(QString("/me "))==0) {
				myTextBrowser->append(tempMsg.replace("/me ","<i>*"+playerName+" ")+"</i>");
			} else if(pm == true) {
				myTextBrowser->append("<i>"+playerName+"(pm): " + tempMsg+"</i>");
			} else {
				myTextBrowser->append(playerName + ": " + tempMsg);
			}
		}
	}
}

void ChatTools::privateMessage(QString playerName, QString message)
{
	bool pm=true;
	receiveMessage(playerName, message, pm);
}

void ChatTools::clearChat()
{

	if(myTextBrowser)
		myTextBrowser->clear();
}

void ChatTools::checkInputLength(QString string)
{

	if(string.toUtf8().length() > 120) myLineEdit->setMaxLength(string.length());
}

void ChatTools::fillChatLinesHistory(QString fillString)
{

	chatLinesHistory << fillString;
	if(chatLinesHistory.size() > 50) chatLinesHistory.removeFirst();


}

void ChatTools::showChatHistoryIndex(int index)
{

	if(index <= chatLinesHistory.size()) {

		// 		cout << chatLinesHistory.size() << " : " <<  index << endl;
		if(index > 0)
			myLineEdit->setText(chatLinesHistory.at(chatLinesHistory.size()-(index)));
		else
			myLineEdit->setText("");
	}
}

void ChatTools::nickAutoCompletition()
{

	QString myChatString = myLineEdit->text();
	QStringList myChatStringList = myChatString.split(" ");

	QStringList matchStringList;

	if(nickAutoCompletitionCounter == 0) {

		if(myNickListModel) {
			int it = 0;
			while (myNickListModel->item(it)) {
				QString text = myNickListModel->item(it, 0)->data(Qt::DisplayRole).toString();
				if(text.startsWith(myChatStringList.last(), Qt::CaseInsensitive) && myChatStringList.last() != "") {
					matchStringList << text;
				}
				++it;
			}
		}

		if(!myNickStringList.isEmpty()) {

			QStringListIterator it(myNickStringList);
			while (it.hasNext()) {
				QString next = it.next();
				if (next.startsWith(myChatStringList.last(), Qt::CaseInsensitive) && myChatStringList.last() != "")
					matchStringList << next;
			}
		}
	}

	if(!matchStringList.isEmpty() || nickAutoCompletitionCounter > 0) {

		myChatStringList.removeLast();

		// 		cout << nickAutoCompletitionCounter << endl;

		if(nickAutoCompletitionCounter == 0) {
			//first one
			lastChatString = myChatStringList.join(" ");
			lastMatchStringList = matchStringList;
		}

		if(nickAutoCompletitionCounter == lastMatchStringList.size()) nickAutoCompletitionCounter = 0;

		// 		cout << nickAutoCompletitionCounter << "\n";

		if(lastChatString == "")
			myLineEdit->setText(lastMatchStringList.at(nickAutoCompletitionCounter)+": ");
		else
			myLineEdit->setText(lastChatString+" "+lastMatchStringList.at(nickAutoCompletitionCounter)+" ");

		nickAutoCompletitionCounter++;
	}
}

void ChatTools::setChatTextEdited()
{

	nickAutoCompletitionCounter = 0;
}

void ChatTools::refreshIgnoreList()
{
	ignoreList = myConfig->readConfigStringList("PlayerIgnoreList");
}

unsigned ChatTools::parsePrivateMessageTarget(QString &chatText)
{
	QString playerName;
	int endPosName = -1;
	// Target player is either in the format "this is a user" or singlename.
	if (chatText.startsWith('"')) {
		chatText.remove(0, 1);
		endPosName = chatText.indexOf('"');
	} else {
		endPosName = chatText.indexOf(' ');
	}
	if (endPosName > 0) {
		playerName = chatText.left(endPosName);
		chatText.remove(0, endPosName + 1);
	}
	chatText = chatText.trimmed();
	unsigned playerId = 0;
	if (!playerName.isEmpty() && !chatText.isEmpty()) {
		if(myNickListModel) {
			int it = 0;
			while (myNickListModel->item(it)) {
				QString text = myNickListModel->item(it, 0)->data(Qt::DisplayRole).toString();
				if(text == playerName) {
					playerId = myNickListModel->item(it, 0)->data(Qt::UserRole).toUInt();
					break;
				}
				++it;
			}
		}
	}
	return playerId;
}

