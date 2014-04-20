#ifndef BUTTONBOXITEM_HPP
#define BUTTONBOXITEM_HPP

#include "stdafx.hpp"

class ButtonBoxItem : public QQuickItem {
    Q_OBJECT
    Q_ENUMS(Button)
    Q_PROPERTY(QQmlComponent *source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(QList<int> buttons READ buttons WRITE setButtons NOTIFY buttonsChanged)
    Q_PROPERTY(qreal buttonWidth READ buttonWidth WRITE setButtonWidth NOTIFY buttonWidthChanged)
    Q_PROPERTY(qreal gap READ gap WRITE setGap NOTIFY gapChanged)
    Q_PROPERTY(QQuickItem *clickedButton READ clickedButton NOTIFY clickedButtonChanged)
    using DBB = QDialogButtonBox;
public:
    enum Button { Ok = DBB::Ok, Cancel = DBB::Cancel, Yes = DBB::Yes, No = DBB::No };
    ButtonBoxItem(QQuickItem *parent = nullptr);
    ~ButtonBoxItem();
    QList<int> buttons() const;
    void setButtons(QList<int> buttons);
    qreal buttonWidth() const;
    qreal gap() const;
    void setButtonWidth(qreal w);
    void setGap(qreal g);
    QQmlComponent *source() const;
    void setSource(QQmlComponent *source);
    QQuickItem *clickedButton() const;
signals:
    void buttonsChanged();
    void gapChanged();
    void buttonWidthChanged();
    void sourceChanged();
    void clicked(int button);
    void clickedButtonChanged();
private slots:
    void emitClicked();
private:
    void updatePolish();
    void geometryChanged(const QRectF &new_, const QRectF &old);
    struct Data;
    Data *d;
};

#endif // BUTTONBOXITEM_HPP