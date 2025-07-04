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

#ifndef MESSAGEWIDGET_H
#define MESSAGEWIDGET_H

#include "messageinfo.h"

#include <vector>

#include <QWidget>
#include <QLabel>
#include <QFontMetrics>
#include <QSizePolicy>
#include <QVBoxLayout>
#include <QDateTime>
#include <QTimer>

class MessageWidget : public QWidget
{
    constexpr static qsizetype MAX_CHARS_PER_LINE = 60;
    constexpr static qsizetype MAX_REPLY_CHARS = 40;
    Q_OBJECT
public:
    explicit MessageWidget(MessageInfo const&,
                           QWidget *parent = nullptr);

    ~MessageWidget();
    void updateText(QString const&);
    void updateUsername(QString const&);
    void updateTimestamp(QString const&);
    void updateReplyText(QString const&);

    void updateInfo(MessageInfo const&);
    int64_t getMsgID();
    QString const& getUsername();
    QString const& getText();
    MessageInfo const& getInfo();

    void resetStyle();
    void setStylePendingAnswer();
    void setStylePendingEdit();
    void setColor(QString const&);
    void highlight(int64_t const, QString const&); // msecs
    int getHeight();


    bool selectedReply = false;
    bool selectedEdit = false;
protected:
    void contextMenuEvent(QContextMenuEvent*) override;
private:
    MessageInfo info;

    QHBoxLayout* mainLayout;
    QVBoxLayout* contentLayout;
    QLabel* headerLabel = nullptr;
    QLabel* lastEditedLabel = nullptr;
    QLabel* replyLabel = nullptr;
    QLabel* isReadLabel = nullptr;

    std::vector<QLabel*> textLabels;
    QWidget* bubbleWidget;

    std::vector<QString> splitTextIntoLines(QString const&);

signals:
    void replyActionSelected(int64_t const, QString, QString);
    void cancelReplySelected(int64_t const);

    void editActionSelected(int64_t const, QString);
    void cancelEditSelected(int64_t const);

    void deleteSelected(int64_t);

    void gotoMsg(int64_t);

    void highlightMsg(int64_t, QString const&);
    void cancelHighlight(int64_t);
};

#endif // MESSAGEWIDGET_H
