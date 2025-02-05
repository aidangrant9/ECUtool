#pragma once

#include <optional>
#include "../communication/SerialConnection.hpp"

class DiagnosticSession
 {
public:
 DiagnosticSession();
 ~DiagnosticSession();

private:
 std::optional<SerialConnection> serialConnection;
 }