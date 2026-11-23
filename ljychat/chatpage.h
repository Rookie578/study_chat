#ifndef CHATPAGE_H
#define CHATPAGE_H

#include <QWidget>

#include "userdata.h"

#include <QMap>
#include <QStringList>
class ChatItemBase;

namespace Ui {
class ChatPage;
}

class ChatPage : public QWidget
{
    Q_OBJECT

public:
    explicit ChatPage(QWidget *parent = nullptr);
    ~ChatPage();

    void SetUserInfo(std::shared_ptr<UserInfo> user_info);
    void AppendChatMsg(std::shared_ptr<TextChatData> msg);

    void UpdateMsgStatus(const QStringList& msgids, int status);
    void UpdateAllMsgStatus(int status);

protected:
    void paintEvent(QPaintEvent *event);

private slots:
    void on_send_btn_clicked();

private:
    Ui::ChatPage *ui;

    std::shared_ptr<UserInfo> _user_info;

    QMap<QString, ChatItemBase*> _msg_items;

signals:
    void sig_append_send_chat_msg(std::shared_ptr<TextChatData> msg);
};

#endif // CHATPAGE_H
