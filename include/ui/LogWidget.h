#ifndef LOG_WIDGET_H
#define LOG_WIDGET_H
#include <QtWidgets/QWidget>
class LogWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LogWidget(QWidget *parent = nullptr);
    ~LogWidget();
};
#endif 