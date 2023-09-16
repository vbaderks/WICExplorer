// Copyright (c) Microsoft Corporation \ Victor Derks.
// SPDX-License-Identifier: MIT

export module EncoderSelectionDlg;

import <std.h>;
import <Windows-import.h>;

export [[nodiscard]] std::optional<std::pair<GUID, GUID>> GetEncoderSelectionFromUser();
