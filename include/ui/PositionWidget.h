#ifndef POSITION_WIDGET_H
#define POSITION_WIDGET_H
#include <QtWidgets/QWidget>
class PositionWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PositionWidget(QWidget *parent = nullptr);
    ~PositionWidget();
};
#endif // POSITION_WIDGET_H 