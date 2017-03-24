; This Source Code Form is subject to the terms of the
; Mozilla Public License, v.2.0. If a copy of the MPL
; was not distributed with this file, You can obtain one
; at http://mozilla.org/MPL/2.0/.

!ifdef BUILD_NUMBER
!define BUILD_SUFFIX ".b${BUILD_NUMBER}"
!else
!define BUILD_SUFFIX ""
!endif

!define PRODUCT_NAME "v8unpack"
!define PRODUCT_VERSION "3.0.40"
!define PRODUCT_VERSION_ID "3.0.40.${BUILD_SUFFIX}"
!define PRODUCT_PUBLISHER "dmpas.ru"
!define PRODUCT_WEB_SITE "http://github.com/dmpas/v8unpack"
!define PRODUCT_DIR_REGKEY "Software\v8unpack"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_EXE "$INSTDIR\v8unpack.exe"
!define BINDIR "Release"
!define RESOURCEDIR "."

BrandingText "$(^NAME) installer (NSIS 2.46)"
InstallDir "$PROGRAMFILES\v8unpack"
Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
SetCompressor /SOLID lzma
ShowInstDetails hide
ShowUnInstDetails hide

OutFile "v8unpack-${PRODUCT_VERSION}${BUILD_SUFFIX}_setup.exe"

!include "LogicLib.nsh"

Section "!Program Files" SEC01
	SectionIn RO 1 2
	SetOverwrite ifnewer

	SetOutPath "$INSTDIR\"
	File "${BINDIR}\v8unpack.exe"
	File "${RESOURCEDIR}\LICENSE"

	SetOutPath "$INSTDIR"

SectionEnd

Section -Post
	WriteUninstaller "$INSTDIR\uninst.exe"
	WriteRegStr SHCTX "${PRODUCT_DIR_REGKEY}" Path "$INSTDIR"
	WriteRegStr SHCTX "${PRODUCT_UNINST_KEY}" "StartMenu" "$SMPROGRAMS\$StartmenuFolder"
	${if} $Answer == "yes" ; if user is admin
		WriteRegStr SHCTX "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
		WriteRegStr SHCTX "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
		WriteRegStr SHCTX "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\v8unpack.exe"
		WriteRegStr SHCTX "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
		WriteRegStr SHCTX "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
		WriteRegStr SHCTX "${PRODUCT_UNINST_KEY}" "URLUpdateInfo" "${PRODUCT_WEB_SITE}"
		WriteRegStr SHCTX "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
		WriteRegDWORD SHCTX "${PRODUCT_UNINST_KEY}" "NoModify" 0x00000001
		WriteRegDWORD SHCTX "${PRODUCT_UNINST_KEY}" "NoRepair" 0x00000001
	${endif}
SectionEnd

Section Uninstall

	RMDir /r "$INSTDIR"
	SetAutoClose true

SectionEnd

