import QtQuick 2.15
import QtQuick.Controls 2.15
import QtCharts 2.15

Item {
    id: root
    width: 600
    height: 400

    property int windowSize: 50    // сколько значений показывать
    property int maxSize: 200      // сколько хранить всего
    property int xOffset: 0        // смещение окна (для прокрутки)

    // массив значений
    property var values: []

    function appendValue(val) {
        // добавляем новое значение
        values.push(val)

        // обрезаем, если превысили maxSize
        if (values.length > maxSize) {
            values.shift()
            if (xOffset > 0) {
                xOffset = Math.max(0, xOffset - 1)
            }
        }

        updateSeries()
    }

    function updateSeries() {
        series.clear()

        let start = xOffset
        let end = Math.min(values.length, xOffset + windowSize)

        for (let i = start; i < end; i++) {
            series.append(i, values[i])
        }

        axisX.min = start
        axisX.max = end - 1
    }

    ChartView {
        id: chart
        anchors.fill: parent
        antialiasing: true

        ValueAxis { id: axisX; min: 0; max: windowSize }
        ValueAxis { id: axisY; min: 0; max: 50 }

        LineSeries {
            id: series
            axisX: axisX
            axisY: axisY
        }
    }

    Slider {
        id: scroll
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        from: 0
        to: Math.max(0, values.length - windowSize)
        stepSize: 1
        value: xOffset
        onValueChanged: {
            xOffset = Math.round(value)
            updateSeries()
        }
    }
}
