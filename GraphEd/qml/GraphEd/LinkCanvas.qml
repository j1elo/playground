import QtQuick 2.0

Rectangle {
    id: root
    color: "grey"

    property alias canvas: linkCanvas
    property list<Link> links

    property Link mouseLink: Link {
        tailX: 0
        tailY: 0
        headX: canvasMouseArea.mouseX
        headY: canvasMouseArea.mouseY
    }
    property bool enableMouseLink: false

    Canvas {
        id: linkCanvas
        anchors.fill: parent
        //renderTarget: Canvas.Image  // Render to an in-memory image buffer.
        //renderTarget: Canvas.FramebufferObject  // Render to an OpenGL frame buffer.

        onPaint: {
            var ctx = linkCanvas.getContext('2d')
            ctx.clearRect(0, 0, linkCanvas.width, linkCanvas.height)

            // Setup line width and stroke color.
            ctx.lineWidth = 4
            ctx.strokeStyle = "#468966"

            // Iterate through the children elements of the
            // container item and draw all of the Socket's links.
            var link
            for (var i = 0; i < links.length; ++i) {
                link = links[i]
                ctx.beginPath()
                ctx.moveTo(link.tailX, link.tailY)
                ctx.lineTo(link.headX, link.headY)
                //ctx.quadraticCurveTo(350, 350, link.headX, link.headY)
                ctx.stroke()
            }

            // Draw link following the mouse cursor.
            if (enableMouseLink) {
                link = mouseLink
                ctx.beginPath()
                ctx.moveTo(link.tailX, link.tailY)
                ctx.lineTo(link.headX, link.headY)
                ctx.stroke()
            }
        }
    }

    MouseArea {
        id: canvasMouseArea
        anchors.fill: parent
        hoverEnabled: enableMouseLink

        // MouseArea::onPositionChanged(MouseEvent mouse)
        onPositionChanged: linkCanvas.requestPaint()
    }
}
