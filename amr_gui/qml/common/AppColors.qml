pragma Singleton
import QtQuick 2.15

QtObject {
    // Backgrounds
    readonly property color backgroundDark:  "#121212"
    readonly property color surfaceDark:     "#1E1E1E"
    readonly property color panelDark:       "#252526"
    readonly property color borderDark:      "#383838"

    readonly property color backgroundLight: "#F5F5F7"  // Soft off-white for the main app background
    readonly property color surfaceLight:    "#FFFFFF"  // Pure white for cards, panels, and toolbars
    readonly property color panelLight:      "#E8E8EC"  // Slightly darkened container / header background
    readonly property color borderLight:     "#D1D1D6"  // Subtle gray divider/border
    readonly property color textBlack:       "#1C1C1E"  // High-contrast primary text for light mode
    readonly property color textDimmedLight: "#6E6E73"

    // Accents & Signals
    readonly property color primaryCyan:     "#00E5FF"
    readonly property color accentRed:       "#FF3366"
    readonly property color buttonNormal:    "#ffffff"
    readonly property color buttonHovered:   "#00B8D4"
    readonly property color buttonPressed:   "#0088A8"

    // Typography
    readonly property color textWhite:       "#FFFFFF"
    readonly property color textDimmed:      "#AAAAAA"
}
