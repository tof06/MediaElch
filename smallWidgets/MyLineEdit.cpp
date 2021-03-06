#include "MyLineEdit.h"

#include <QDebug>
#include <QMovie>
#include <QPainter>
#include <QStyle>
#include <QToolButton>

#include "globals/Globals.h"
#include "globals/Helper.h"

/**
 * @brief MyLineEdit::MyLineEdit
 * @param parent
 */
MyLineEdit::MyLineEdit(QWidget *parent) :
    QLineEdit(parent)
{
    m_showMagnifier = false;
    m_magnifierLabel = 0;
    m_paddingLeft = 0;
    m_moreLabel = new QLabel(this);
    m_moreLabel->setText("...");
    m_moreLabel->setStyleSheet("font-size: 10px; color: #a0a0a0;");
    m_moreLabel->hide();
    m_loadingLabel = new QLabel(0);
    m_loadingLabel->hide();
    connect(this, SIGNAL(textChanged(QString)), this, SLOT(myTextChanged(QString)));
}

/**
 * @brief Moves the icons to their positions
 */
void MyLineEdit::resizeEvent(QResizeEvent *)
{
    if (m_type == TypeLoading) {
        QSize size = m_loadingLabel->sizeHint();
        int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
        m_loadingLabel->move(rect().right()-frameWidth-size.width(), (rect().bottom()+1-size.height())/2);
    } else if (m_type == TypeClear) {
        QSize size = m_clearButton->sizeHint();
        int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
        m_clearButton->move(rect().right()-frameWidth-size.width()+2, (rect().bottom()+1-size.height()+6)/2);
    }

    if (m_showMagnifier) {
        QSize size = m_magnifierLabel->sizeHint();
        m_magnifierLabel->move(6, (rect().bottom()+1-size.height())/2);
    }
}

/**
 * @brief Captures key events and emits signals based on the key
 * @param event
 */
void MyLineEdit::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Down) {
        emit keyDown();
        return;
    }
    if (event->key() == Qt::Key_Up) {
        emit keyUp();
        return;
    }
    if (event->key() == Qt::Key_Backspace && cursorPosition() == 0) {
        emit backspaceInFront();
    }
    QLineEdit::keyPressEvent(event);
}

/**
 * @brief Emits custom focusOut signal
 * @param event
 */
void MyLineEdit::focusOutEvent(QFocusEvent *event)
{
    emit focusOut();
    QLineEdit::focusOutEvent(event);
}

/**
 * @brief Emits custom focusIn signal
 * @param event
 */
void MyLineEdit::focusInEvent(QFocusEvent *event)
{
    emit focusIn();
    QLineEdit::focusInEvent(event);
}

/**
 * @brief Shows/hides the loading movie and disabled/enables the line edit
 * @param loading Is loading
 */
void MyLineEdit::setLoading(bool loading)
{
    if (m_type != TypeLoading)
        return;
    m_loadingLabel->setVisible(loading);
    QLineEdit::setDisabled(loading);
}

/**
 * @brief Sets the type of the line edit
 * @param type Type of the line edit
 */
void MyLineEdit::setType(LineEditType type)
{
    m_type = type;

    if (type == TypeLoading) {
        m_loadingLabel->deleteLater();
        m_loadingLabel = new QLabel(this);
        QMovie *movie = new QMovie(":/img/spinner.gif");
        movie->start();
        m_loadingLabel->setMovie(movie);
        QSize minimumSize = minimumSizeHint();
        int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
        m_styleSheets.append(QString("QLineEdit { padding-right: %2px; }").arg(m_loadingLabel->sizeHint().width() + frameWidth + 1));
        setStyleSheet(m_styleSheets.join(" ") + QString(" QLineEdit { padding-left: %1px; }").arg(m_paddingLeft));
        setMinimumSize(qMax(minimumSize.width(), m_loadingLabel->sizeHint().width() + frameWidth * 2 + 2),
                             qMax(minimumSize.height(), m_loadingLabel->sizeHint().height() + frameWidth * 2 + 2));
        m_loadingLabel->setHidden(true);
    }

    if (type == TypeClear) {
        m_clearButton = new QToolButton(this);
        m_clearButton->setFixedSize(14, 14);
        m_clearButton->setCursor(Qt::ArrowCursor);
        m_clearButton->setIcon(QIcon(":/img/stop.png"));
        m_clearButton->setStyleSheet("background-color: transparent; border: none;");
        QSize minimumSize = minimumSizeHint();
        int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
        m_styleSheets.append(QString("QLineEdit { padding-right: %2px; } ").arg(m_clearButton->sizeHint().width() + frameWidth + 1));
        setStyleSheet(m_styleSheets.join(" ") + QString(" QLineEdit { padding-left: %1px; }").arg(m_paddingLeft));
        setMinimumSize(qMax(minimumSize.width(), m_clearButton->sizeHint().width() + frameWidth * 2 + 2),
                             (12 + frameWidth * 2 + 2));
        m_clearButton->setVisible(!text().isEmpty());
        connect(m_clearButton, SIGNAL(clicked()), this, SLOT(myClear()), Qt::UniqueConnection);
    }
}

/**
 * @brief Returns the type of the line edit
 * @return Type of the line edit
 */
MyLineEdit::LineEditType MyLineEdit::type()
{
    return m_type;
}

/**
 * @brief Shows/hides the clear button
 * @param text Current text
 */
void MyLineEdit::myTextChanged(QString text)
{
    if (m_type != TypeClear)
        return;

    m_clearButton->setVisible(!text.isEmpty());
}

/**
 * @brief Clears the text
 */
void MyLineEdit::myClear()
{
    setText("");
}

/**
 * @brief Sets and additional style sheet
 * @param style Stylesheet
 */
void MyLineEdit::addAdditionalStyleSheet(QString style)
{
    m_styleSheets.append(style);
}

/**
 * @brief Show/hide the magnifier
 * @param show
 */
void MyLineEdit::setShowMagnifier(bool show)
{
    if (m_showMagnifier && !show)
        m_paddingLeft -= 24;
    else if (!m_showMagnifier && show)
        m_paddingLeft += 24;

    m_showMagnifier = show;

    if (show) {
        if (m_magnifierLabel != 0)
            delete m_magnifierLabel;
        QPixmap magn(":/img/magnifier.png");
        magn = magn.scaled(QSize(14, 14) * Helper::devicePixelRatio(this), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        Helper::setDevicePixelRatio(magn, Helper::devicePixelRatio(this));
        QPainter p;
        p.begin(&magn);
        p.setCompositionMode(QPainter::CompositionMode_SourceIn);
        p.fillRect(magn.rect(), QColor(0, 0, 0, 150));
        p.end();

        m_magnifierLabel = new QLabel(this);
        m_magnifierLabel->setPixmap(magn);
        m_magnifierLabel->setStyleSheet("border: none;");
        setStyleSheet(m_styleSheets.join(" ") + QString(" QLineEdit { padding-left: %1px; }").arg(m_paddingLeft));
    }
}

/**
 * @brief Adds a filter and clears text
 * @param filter
 */
void MyLineEdit::addFilter(Filter *filter)
{
    QLabel *label = new QLabel(this);
    if (filter->info() == MovieFilters::Title || filter->info() == MovieFilters::Path || filter->info() == ConcertFilters::Title || filter->info() == TvShowFilters::Title)
        label->setStyleSheet("background-color: #999999; border: 1px solid #999999; border-radius: 2px; font-size: 10px; color: #ffffff;");
    else if (filter->info() == MovieFilters::ImdbId)
        label->setStyleSheet("background-color: #F0AD4E; border: 1px solid #F0AD4E; border-radius: 2px; font-size: 10px; color: #ffffff;");
    else
        label->setStyleSheet("background-color: #5BC0DE; border: 1px solid #5BC0DE; border-radius: 2px; font-size: 10px; color: #ffffff;");
    label->setText(filter->shortText());
    label->show();
    m_filterLabels.append(label);
    drawFilters();
    setText("");
}

/**
 * @brief Removes the last filter
 */
void MyLineEdit::removeLastFilter()
{
    if (m_filterLabels.count() == 0)
        return;
    m_filterLabels.takeLast()->deleteLater();
    drawFilters();
}

/**
 * @brief Clears all filters
 */
void MyLineEdit::clearFilters()
{
    foreach (QLabel *label, m_filterLabels)
        label->deleteLater();
    m_filterLabels.clear();
    drawFilters();
}

/**
 * @brief Draws the filter labels
 *        If necessary this also show the "..." label in front of the filter labels
 */
void MyLineEdit::drawFilters()
{
    int paddingLeft = m_paddingLeft;
    int labelWidth = 0;
    int hidden = 0;
    foreach (QLabel *l, m_filterLabels) {
        labelWidth += l->width()+2;
        l->show();
    }
    while (labelWidth+50 > width() && hidden<m_filterLabels.count()) {
        m_filterLabels.at(hidden++)->hide();
        labelWidth = 0;
        foreach (QLabel *l, m_filterLabels) {
            if (l->isVisible())
                labelWidth += l->width()+2;
        }
    }

    if (hidden > 0) {
        m_moreLabel->move(m_paddingLeft, 1);
        m_moreLabel->show();
        paddingLeft += m_moreLabel->width();
    } else {
        m_moreLabel->hide();
    }

    foreach (QLabel *l, m_filterLabels) {
        if (l->isVisible() || l == m_filterLabels.last()) {
            l->move(paddingLeft, 1);
            paddingLeft += l->width()+2;
        }
    }

    setStyleSheet(m_styleSheets.join(" ") + QString(" QLineEdit { padding-left: %1px; }").arg(paddingLeft));
}

/**
 * @brief Returns the current left padding. The padding is calculated by
 *        summing up all visible filter labels
 * @return Current offset from left
 */
int MyLineEdit::paddingLeft()
{
    int paddingLeft = m_paddingLeft;
    if (m_moreLabel->isVisible())
        paddingLeft += m_moreLabel->width();
    foreach (QLabel *l, m_filterLabels) {
        if (l->isVisible())
            paddingLeft += l->width()+2;
    }
    return paddingLeft;
}
