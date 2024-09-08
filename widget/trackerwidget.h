#pragma once

#include <QWidget>
#include <QPainter>
#include <QObjectBindableProperty>

class TrackerWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QVector<QStringList> patternContent
               READ patternContent WRITE setPatternContent BINDABLE bindablePatternContent)
    Q_PROPERTY(int currentRow
               READ currentRow WRITE setCurrentRow BINDABLE bindableCurrentRow)

public:
    explicit TrackerWidget(QWidget *parent = nullptr);
    ~TrackerWidget();

    QVector<QStringList> patternContent() { return m_patternContent.value(); }
    void setPatternContent(const QVector<QStringList> & content) { m_patternContent = content; }
    QBindable<QVector<QStringList>> bindablePatternContent() { return &m_patternContent; }

    int currentRow() { return m_CurrentRow.value(); }
    void setCurrentRow(int content) { m_CurrentRow = content; }
    QBindable<int> bindableCurrentRow() { return &m_CurrentRow; }

protected:
    void paintEvent(QPaintEvent *event) override;

signals:
    void patternContentChanged();
    void currentRowChanged(int row);

private:
    Q_OBJECT_BINDABLE_PROPERTY(TrackerWidget, QVector<QStringList>, m_patternContent, &TrackerWidget::patternContentChanged);
    Q_OBJECT_BINDABLE_PROPERTY(TrackerWidget, int, m_CurrentRow, &TrackerWidget::currentRowChanged);
};
