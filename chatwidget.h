// Copyright 2025 Medvedev Dan (https://github.com/Firegreat78)

/*
Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#ifndef CHATWIDGET_H
#define CHATWIDGET_H

#include "messagewidget.h"
#include "messageinfo.h"

#include <QWidget>
#include <QTimer>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QMap>

#include <map>
#include <vector>

class ChatWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ChatWidget(QWidget *parent = nullptr);

    void addMessage(MessageInfo const&, bool const);
    void removeMessage(int64_t const);

    void pop_back();
    void pop_front();

    void resetFilter();

    MessageInfo const& getMessageInfo(int64_t);

    void updateInfo(int64_t const, MessageInfo const&);

    void scrollToBottom(int64_t const);
    void scrollToMid(int64_t const);
    void scroll(int64_t const, double const);

    qint64 messageAmount();
    int64_t firstMessageID();
    int64_t lastMessageID();
    std::vector<int64_t> getCurrentMessagesID();
    std::vector<int64_t> getVisibleMessagesID();

    // These two are for msg preloading
    // when a user scrolls up or down
    bool isScrolledUp();
    bool isScrolledDown();

    void resetPendingAnswerMsg();
    void resetPendingEditMsg();
    int64_t getMsgPendingAnswer();
    int64_t getMsgPendingEdit();

    // Whether the msg is rendered on the chat widget
    // if it's scrolled out of view, it's not visible
    bool isMsgVisible(int64_t);

    // Whether the chat has a particular msg
    // (even if it's scrolled out of view)
    bool isMsgPresent(int64_t);

    bool isMsgHighlighted(int64_t);

    void gotoMessage(int64_t, int64_t);
    void highlight(int64_t, int);

    // This does not reset pending reply to/edit msg ids.
    void clear();

private slots:
    void onSelectedMsgAnswer(int64_t, QString, QString);
    void onCancelAnswering(int64_t);

    void onSelectedMsgEdit(int64_t, QString);
    void onCancelEdit(int64_t);

    void onDeleteMsg(int64_t);

    void cancelHighlight();
public slots:

private:
    QVBoxLayout *m_layout;
    QWidget *m_contentWidget;
    QScrollArea *m_scrollArea;
    std::map<int64_t, MessageWidget*> messages;
    int64_t msgPendingAnswer = 0;
    int64_t msgPendingEdit = 0;

    int64_t msgHighlighted = 0;
    QTimer* highlightTimer = nullptr;

    QString filterContent = "";
    bool isFilterDtEnabled = false;
    QDateTime filterDtBegin;
    QDateTime filterDtEnd;
signals:
};

#endif // CHATWIDGET_H
