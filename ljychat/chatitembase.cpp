
#include "chatitembase.h"



ChatItemBase::ChatItemBase(ChatRole role, QWidget *parent):QWidget(parent),m_role(role)
{
    //NameLabel
    m_pNameLabel    = new QLabel();
    m_pNameLabel->setObjectName("chat_user_name");
    QFont font("Microsoft YaHei");
    font.setPointSize(9);
    m_pNameLabel->setFont(font);
    m_pNameLabel->setFixedHeight(20);

    //IconLabel
    m_pIconLabel    = new QLabel();
    m_pIconLabel->setScaledContents(true);
    m_pIconLabel->setFixedSize(42, 42);

    m_pBubble       = new QWidget();

    m_pStatusLabel = new QLabel();
    m_pStatusLabel->setObjectName("chat_status");
    m_pStatusLabel->setStyleSheet("color: gray; font-size: 10px;");
    m_pStatusLabel->setContentsMargins(0,0,8,0);
    m_pStatusLabel->setAlignment(Qt::AlignRight);
    m_pStatusLabel->hide();

    //网格
    QGridLayout *pGLayout = new QGridLayout();
    pGLayout->setVerticalSpacing(3); //控件间的垂直间距
    pGLayout->setHorizontalSpacing(3); //水平间距
    pGLayout->setMargin(3); // 组件和边界之间的间距

    //弹簧
    QSpacerItem*pSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    //判断角色
    if(m_role == ChatRole::Self)
    {
        m_pNameLabel->setContentsMargins(0,0,8,0);
        m_pNameLabel->setAlignment(Qt::AlignRight);
        pGLayout->addWidget(m_pNameLabel, 0,1, 1,1);
        pGLayout->addWidget(m_pIconLabel, 0, 2, 2,1, Qt::AlignTop);
        pGLayout->addItem(pSpacer, 1, 0, 1, 1);
        pGLayout->addWidget(m_pBubble, 1,1, 1,1);

        pGLayout->addWidget(m_pStatusLabel, 2, 1, 1, 1, Qt::AlignRight);

        pGLayout->setColumnStretch(0, 2);
        pGLayout->setColumnStretch(1, 3);
    }else{
        m_pNameLabel->setContentsMargins(8,0,0,0);
        m_pNameLabel->setAlignment(Qt::AlignLeft);
        pGLayout->addWidget(m_pIconLabel, 0, 0, 2,1, Qt::AlignTop);
        pGLayout->addWidget(m_pNameLabel, 0,1, 1,1);
        pGLayout->addWidget(m_pBubble, 1,1, 1,1);
        pGLayout->addItem(pSpacer, 1, 2, 1, 1);

        pGLayout->setColumnStretch(1, 3);
        pGLayout->setColumnStretch(2, 2);
    }

    this->setLayout(pGLayout);
}

void ChatItemBase::setUserName(const QString &name)
{
    m_pNameLabel->setText(name);
}

void ChatItemBase::setUserIcon(const QPixmap &icon)
{
    m_pIconLabel->setPixmap(icon);
}

void ChatItemBase::setWidget(QWidget *w)
{
    QGridLayout *pGLayout = (qobject_cast<QGridLayout *>)(this->layout());
    pGLayout->replaceWidget(m_pBubble, w);
    delete m_pBubble;
    m_pBubble = w;
}

void ChatItemBase::setStatus(const QString &text)
{
    m_pStatusLabel->setText(text);
    m_pStatusLabel->setVisible(!text.isEmpty());
}


















