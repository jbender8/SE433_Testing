; Script generated with the Venis Install Wizard

; Define your application name
!define APPNAME "Notepad++"
!define APPNAMEANDVERSION "Notepad++ 2.5"

; Main Install settings
Name "${APPNAMEANDVERSION}"
InstallDir "$PROGRAMFILES\Notepad++"
InstallDirRegKey HKLM "Software\${APPNAME}" ""
OutFile "npp.2.5.Installer.exe"


; Modern interface settings
!include "MUI.nsh"

!define MUI_ABORTWARNING
!define MUI_FINISHPAGE_RUN "$INSTDIR\notepad++.exe"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\license.txt"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

; Set languages (first is default language)
;!insertmacro MUI_LANGUAGE "English"

;Languages

  !insertmacro MUI_LANGUAGE "English"
  !insertmacro MUI_LANGUAGE "French"
  !insertmacro MUI_LANGUAGE "TradChinese"
  !insertmacro MUI_LANGUAGE "Spanish"
  !insertmacro MUI_LANGUAGE "Hungarian"
  !insertmacro MUI_LANGUAGE "Russian"
  !insertmacro MUI_LANGUAGE "German"
  !insertmacro MUI_LANGUAGE "Dutch"
  !insertmacro MUI_LANGUAGE "SimpChinese"
  !insertmacro MUI_LANGUAGE "Italian"
  !insertmacro MUI_LANGUAGE "Danish"
  !insertmacro MUI_LANGUAGE "Polish"
  !insertmacro MUI_LANGUAGE "Czech"
  !insertmacro MUI_LANGUAGE "Slovenian"
  !insertmacro MUI_LANGUAGE "Slovak"
  !insertmacro MUI_LANGUAGE "Swedish"
  !insertmacro MUI_LANGUAGE "Norwegian"
  !insertmacro MUI_LANGUAGE "PortugueseBR"
  !insertmacro MUI_LANGUAGE "Ukrainian"
  !insertmacro MUI_LANGUAGE "Turkish"
  !insertmacro MUI_LANGUAGE "Catalan"
  
  ;!insertmacro MUI_LANGUAGE "Japanese"
  ;!insertmacro MUI_LANGUAGE "Korean"
  ;!insertmacro MUI_LANGUAGE "Finnish"
  ;!insertmacro MUI_LANGUAGE "Greek"
  ;!insertmacro MUI_LANGUAGE "Portuguese"
  ;!insertmacro MUI_LANGUAGE "Croatian"
  ;!insertmacro MUI_LANGUAGE "Bulgarian"
  ;!insertmacro MUI_LANGUAGE "Thai"
  ;!insertmacro MUI_LANGUAGE "Romanian"
  ;!insertmacro MUI_LANGUAGE "Latvian"
  ;!insertmacro MUI_LANGUAGE "Macedonian"
  ;!insertmacro MUI_LANGUAGE "Estonian"
  ;!insertmacro MUI_LANGUAGE "Lithuanian"
  ;!insertmacro MUI_LANGUAGE "Serbian"
  ;!insertmacro MUI_LANGUAGE "Arabic"
  ;!insertmacro MUI_LANGUAGE "Farsi"
  ;!insertmacro MUI_LANGUAGE "Hebrew"
  ;!insertmacro MUI_LANGUAGE "Indonesian"

!insertmacro MUI_RESERVEFILE_LANGDLL

;Installer Functions

!macro NPP_LANGDLL_DISPLAY

  ;!verbose push

  !insertmacro MUI_DEFAULT MUI_LANGDLL_WINDOWTITLE "Notepad++ Installer"
  !insertmacro MUI_DEFAULT MUI_LANGDLL_INFO "Please select a language."
  
  LangDLL::LangDialog "${MUI_LANGDLL_WINDOWTITLE}" "${MUI_LANGDLL_INFO}" A ${MUI_LANGDLL_PUSHLIST} ""

  Pop $LANGUAGE
  StrCmp $LANGUAGE "cancel" 0 +2
    Abort
		
  ;!verbose pop

!macroend

Function .onInit

  !insertmacro NPP_LANGDLL_DISPLAY

FunctionEnd

LangString langFileName ${LANG_ENGLISH} "english.xml"
LangString langFileName ${LANG_FRENCH} "french.xml"
LangString langFileName ${LANG_TRADCHINESE} "chinese.xml"
LangString langFileName ${LANG_GERMAN} "german.xml"
LangString langFileName ${LANG_SPANISH} "spanish.xml"
LangString langFileName ${LANG_HUNGARIAN} "hungarian.xml"
LangString langFileName ${LANG_RUSSIAN} "russian.xml"
LangString langFileName ${LANG_DUTCH} "dutch.xml"
LangString langFileName ${LANG_SIMPCHINESE} "chineseSimplified.xml"
LangString langFileName ${LANG_ITALIAN} "italian.xml"
LangString langFileName ${LANG_DANISH} "danish.xml"
LangString langFileName ${LANG_POLISH} "polish.xml"
LangString langFileName ${LANG_CZECH} "czech.xml"
LangString langFileName ${LANG_SLOVENIAN} "slovenian.xml"
LangString langFileName ${LANG_SLOVAK} "slovak.xml"
LangString langFileName ${LANG_SWEDISH} "swedish.xml"
LangString langFileName ${LANG_NORWEGIAN} "norwegian.xml"
LangString langFileName ${LANG_PORTUGUESEBR} "brazilian_portuguese.xml"
LangString langFileName ${LANG_UKRAINIAN} "ukrainian.xml"
LangString langFileName ${LANG_TURKISH} "turkish.xml"
LangString langFileName ${LANG_CATALAN} "catalan.xml"

; Modern install component descriptions
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
	;!insertmacro MUI_DESCRIPTION_TEXT ${mainSection} ""
!insertmacro MUI_FUNCTION_DESCRIPTION_END

Section -"Notepad++" mainSection

	; Set Section properties
	SetOverwrite on

	; Set Section Files and Shortcuts
	SetOutPath "$INSTDIR\"
	File "LINEDRAW.TTF"
	File "SciLexer.dll"
	File "change.log"
	File "config.xml"
	File "langs.xml"
	File "notepad++.exe"
	File "readme.txt"
	File "stylers.xml"
	
	SetOutPath "$APPDATA\Notepad++"
	File "stylers.xml"
	File "config.xml"
	
	StrCmp $LANGUAGE ${LANG_ENGLISH} noLang 0
	StrCmp $LANGUAGE ${LANG_FRENCH} 0 +3
		File ".\nativeLang\french.xml"
		Goto finLang
	StrCmp $LANGUAGE ${LANG_TRADCHINESE} 0 +3
		File ".\nativeLang\chinese.xml"
		Goto finLang
	StrCmp $LANGUAGE ${LANG_SPANISH} 0 +3
		File ".\nativeLang\spanish.xml"
		Goto finLang
	StrCmp $LANGUAGE ${LANG_HUNGARIAN} 0 +3
		File ".\nativeLang\hungarian.xml"
		Goto finLang
	StrCmp $LANGUAGE ${LANG_RUSSIAN} 0 +3
		File ".\nativeLang\russian.xml"
		Goto finLang
	StrCmp $LANGUAGE ${LANG_GERMAN} 0 +3
		File ".\nativeLang\german.xml"
		Goto finLang
		
	StrCmp $LANGUAGE ${LANG_DUTCH} 0 +3
		File ".\nativeLang\dutch.xml"
		Goto finLang
	StrCmp $LANGUAGE ${LANG_SIMPCHINESE} 0 +3
		File ".\nativeLang\chineseSimplified.xml"
		Goto finLang
	StrCmp $LANGUAGE ${LANG_ITALIAN} 0 +3
		File ".\nativeLang\italian.xml"
		Goto finLang
	StrCmp $LANGUAGE ${LANG_DANISH} 0 +3
		File ".\nativeLang\danish.xml"
		Goto finLang
	StrCmp $LANGUAGE ${LANG_POLISH} 0 +3
		File ".\nativeLang\polish.xml"
		Goto finLang
	StrCmp $LANGUAGE ${LANG_CZECH} 0 +3
		File ".\nativeLang\czech.xml"
		Goto finLang
	StrCmp $LANGUAGE ${LANG_SLOVENIAN} 0 +3
		File ".\nativeLang\slovenian.xml"
		Goto finLang
	StrCmp $LANGUAGE ${LANG_SLOVAK} 0 +3
		File ".\nativeLang\slovak.xml"
		Goto finLang
		
	StrCmp $LANGUAGE ${LANG_SWEDISH} 0 +3
		File ".\nativeLang\swedish.xml"
		Goto finLang
	StrCmp $LANGUAGE ${LANG_NORWEGIAN} 0 +3
		File ".\nativeLang\norwegian.xml"
		Goto finLang
	StrCmp $LANGUAGE ${LANG_PORTUGUESEBR} 0 +3
		File ".\nativeLang\brazilian_portuguese.xml"
		Goto finLang
	StrCmp $LANGUAGE ${LANG_UKRAINIAN} 0 +3
		File ".\nativeLang\ukrainian.xml"
		Goto finLang
	StrCmp $LANGUAGE ${LANG_TURKISH} 0 +3
		File ".\nativeLang\turkish.xml"
		Goto finLang
	StrCmp $LANGUAGE ${LANG_CATALAN} 0 +3
		File ".\nativeLang\catalan.xml"
		Goto finLang	
		
	finLang:
	IfFileExists "$APPDATA\Notepad++\nativeLang.xml" 0 +2
		Delete "$APPDATA\Notepad++\nativeLang.xml"
	
	Rename "$APPDATA\Notepad++\$(langFileName)" "$APPDATA\Notepad++\nativeLang.xml"
	Goto commun
	
	noLang:
	IfFileExists "$APPDATA\Notepad++\nativeLang.xml" 0 +2
		Delete "$APPDATA\Notepad++\nativeLang.xml"
	
	commun:
	CreateShortCut "$DESKTOP\Notepad++.lnk" "$INSTDIR\notepad++.exe"
	CreateDirectory "$SMPROGRAMS\Notepad++"
	CreateShortCut "$SMPROGRAMS\Notepad++\Notepad++.lnk" "$INSTDIR\notepad++.exe"
	CreateShortCut "$SMPROGRAMS\Notepad++\Uninstall.lnk" "$INSTDIR\uninstall.exe"

SectionEnd

SubSection /e "Auto-completion Files" autoCompletionComponent
	SetOverwrite off

	Section PHP
		SetOutPath "$INSTDIR\plugins\APIs"
		File ".\plugins\APIs\php.api"
	SectionEnd
	
	Section CSS
		SetOutPath "$INSTDIR\plugins\APIs"
		File ".\plugins\APIs\css.api"
	SectionEnd
	
	Section VB
		SetOutPath "$INSTDIR\plugins\APIs"
		File ".\plugins\APIs\vb.api"
	SectionEnd
	
SubSectionEnd

Section "Context menu entry" contextMenu
	WriteRegStr HKCR "*\shell\Notepad++\Command" "" "$\"$INSTDIR\notepad++.exe$\" $\"%1$\""
SectionEnd

Section "As default html viewer" htmlViewer
	SetOutPath "$INSTDIR\"
	File "nppIExplorerShell.exe"
	WriteRegStr HKLM "SOFTWARE\Microsoft\Internet Explorer\View Source Editor\Editor Name" "" "$INSTDIR\nppIExplorerShell.exe"
SectionEnd

Section -FinishSection

	WriteRegStr HKLM "Software\${APPNAME}" "" "$INSTDIR"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "DisplayName" "${APPNAME}"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "UninstallString" "$INSTDIR\uninstall.exe"
	WriteUninstaller "$INSTDIR\uninstall.exe"

SectionEnd



;Uninstall section

SubSection un.autoCompletionComponent
	Section un.PHP
		Delete "$INSTDIR\plugins\APIs\php.api"
		RMDir "$INSTDIR\plugins\APIs\"
	SectionEnd
	
	Section un.CSS
		Delete "$INSTDIR\plugins\APIs\css.api"
		RMDir "$INSTDIR\plugins\APIs\"
	SectionEnd
	
	Section un.VB
		Delete "$INSTDIR\plugins\APIs\vb.api"
		RMDir "$INSTDIR\plugins\APIs\"
	SectionEnd
	
SubSectionEnd

Section Uninstall

	;Remove from registry...
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}"
	DeleteRegKey HKLM "SOFTWARE\${APPNAME}"

	; Delete self
	Delete "$INSTDIR\uninstall.exe"

	; Delete Shortcuts
	Delete "$DESKTOP\Notepad++.lnk"
	Delete "$SMPROGRAMS\Notepad++\Notepad++.lnk"
	Delete "$SMPROGRAMS\Notepad++\Uninstall.lnk"

	; Clean up Notepad++
	Delete "$INSTDIR\LINEDRAW.TTF"
	Delete "$INSTDIR\SciLexer.dll"
	Delete "$INSTDIR\change.log"
	Delete "$INSTDIR\config.xml"
	Delete "$INSTDIR\langs.xml"
	Delete "$INSTDIR\notepad++.exe"
	Delete "$INSTDIR\readme.txt"
	Delete "$INSTDIR\stylers.xml"
	Delete "$INSTDIR\nppIExplorerShell.xml"
	
	Delete "$APPDATA\Notepad++\stylers.xml"
	Delete "$APPDATA\Notepad++\config.xml"
	
	Delete "$APPDATA\Notepad++\nativeLang.xml"

	
	; Remove remaining directories
	RMDir "$SMPROGRAMS\Notepad++"
	RMDir "$INSTDIR\"
	RMDir "$APPDATA\Notepad++"

SectionEnd
;--------------------------------
;Uninstaller Functions
Section un.htmlViewer
	DeleteRegKey HKLM "SOFTWARE\Microsoft\Internet Explorer\View Source Editor"
SectionEnd

Section un.contextMenu
	DeleteRegKey HKCR "*\shell\Notepad++"
SectionEnd


Function un.onInit

  !insertmacro MUI_UNGETLANGUAGE
  
FunctionEnd

BrandingText "Don HO - Le chef de projet à lui tout seul"

; eof