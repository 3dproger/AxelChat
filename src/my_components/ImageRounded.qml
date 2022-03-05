import QtQuick 2.15
import QtQuick.Controls 2.15

Image {
    property bool rounded: true
    property bool adapt: true

    fillMode: Image.PreserveAspectFit

    layer.enabled: rounded
    layer.effect: ShaderEffect {
        property real adjustX: adapt ? Math.max(width / height, 1) : 1
        property real adjustY: adapt ? Math.max(1 / (width / height), 1) : 1

        fragmentShader: "
        #extension GL_OES_standard_derivatives : enable
        #ifdef GL_ES
            precision lowp float;
        #endif // GL_ES
        varying highp vec2 qt_TexCoord0;
        uniform highp float qt_Opacity;
        uniform lowp sampler2D source;
        uniform lowp float adjustX;
        uniform lowp float adjustY;

        void main(void) {
            lowp float x, y;
            x = (qt_TexCoord0.x - 0.5) * adjustX;
            y = (qt_TexCoord0.y - 0.5) * adjustY;
            float delta = adjustX != 1.0 ? fwidth(y) / 2.0 : fwidth(x) / 2.0;
            gl_FragColor = texture2D(source, qt_TexCoord0).rgba
                * step(x * x + y * y, 0.25)
                * smoothstep((x * x + y * y) , 0.25 + delta, 0.25)
                * qt_Opacity;
        }"
    }
}
