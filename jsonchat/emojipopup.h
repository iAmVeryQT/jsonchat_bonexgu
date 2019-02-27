#ifndef EMOJIPOPUP_H
#define EMOJIPOPUP_H

#include <QDialog>
#include <QTableWidgetItem>

namespace Ui {
class EmojiPopup;
}

class EmojiPopup : public QDialog
{
    Q_OBJECT

public:
    explicit EmojiPopup(QWidget *parent = 0);
    ~EmojiPopup();

private:
    void focusOutEvent(QFocusEvent*) Q_DECL_OVERRIDE;

private slots:
    void on_itemPressed(QTableWidgetItem* item);

private:
    Ui::EmojiPopup *ui;
};

#endif // EMOJIPOPUP_H
