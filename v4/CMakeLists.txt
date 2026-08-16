cmake_minimum_required(VERSION 3.20)
project(llama-ui-v4 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

# Qt6 Widgets + Network only. No WebEngine, no Quick — nothing that needs
# Qt environment variables (QT_QPA_PLATFORM, QTWEBENGINE_*, …). The app runs
# with a plain environment; theme and DPI handling are done in code.
find_package(Qt6 REQUIRED COMPONENTS Widgets Network Svg)

add_executable(llama-ui-v4
  src/main.cpp
  src/MainWindow.cpp
  src/MainWindow.h
  src/Theme.cpp
  src/Theme.h
  src/Api.cpp
  src/Api.h
  src/i18n.cpp
  src/i18n.h
  src/Icons.cpp
  src/Icons.h
  src/Config.cpp
  src/Config.h
  src/ConversationStore.cpp
  src/ConversationStore.h
  src/Presets.cpp
  src/Presets.h
  src/widgets/Sidebar.cpp
  src/widgets/Sidebar.h
  src/widgets/Composer.cpp
  src/widgets/Composer.h
  src/widgets/MessageView.cpp
  src/widgets/MessageView.h
  src/widgets/SettingsDialog.cpp
  src/widgets/SettingsDialog.h
  src/widgets/WizardDialog.cpp
  src/widgets/WizardDialog.h
  src/widgets/ContextGauge.cpp
  src/widgets/ContextGauge.h
  src/widgets/ModelsSelector.cpp
  src/widgets/ModelsSelector.h
  src/widgets/McpPage.cpp
  src/widgets/McpPage.h
)

target_link_libraries(llama-ui-v4 PRIVATE Qt6::Widgets Qt6::Network Qt6::Svg)

# i18n dicts + spec are read at runtime from the repo (frontend/v2/lang).
target_compile_definitions(llama-ui-v4 PRIVATE
  UI_SPEC_DIR="${CMAKE_SOURCE_DIR}/.."
  APP_VERSION="0.5.1"
  APP_BUILD="0x07D23"
)
