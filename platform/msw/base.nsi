; Shoes definitions
!define SHOES_NAME "#{APPNAME}"
!define SHOES_VERSION "#{WINVERSION}"
!define SHOES_PUBLISHER "shoesrb"
!define SHOES_WEB_SITE "http://shoesrb.com/"
!define SHOES_INST_KEY "Software\Hackety.org\${SHOES_NAME}"
!define SHOES_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${SHOES_NAME}"
!define SHOES_UNINST_ROOT_KEY "HKLM"

SetCompressor /SOLID lzma

!include "MUI.nsh"
!include "LogicLib.nsh"
!include "FileFunc.nsh"
!include "FileAssociation.nsh"
!include "EnvVarUpdate.nsh"

Var StartMenuFolder
Var UninstallerHasFinished

; MUI Settings
!define MUI_ABORTWARNING
!define MUI_ICON setup.ico
!define MUI_UNICON setup.ico
!define MUI_WELCOMEPAGE_TITLE_3LINES
!define MUI_WELCOMEFINISHPAGE_BITMAP installer-1.bmp
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_RIGHT
!define MUI_HEADERIMAGE_BITMAP installer-2.bmp
!define MUI_COMPONENTSPAGE_NODESC

; MUI Pages
!insertmacro MUI_PAGE_WELCOME
!define MUI_PAGE_HEADER_TEXT "Shoes is MIT Licensed"
!define MUI_PAGE_HEADER_SUBTEXT "You are free to do as you please with Shoes."
!define MUI_LICENSEPAGE_TEXT_TOP "When you are ready to continue with Setup, click I Agree."
!define MUI_LICENSEPAGE_TEXT_BOTTOM " "
!insertmacro MUI_PAGE_LICENSE "..\COPYING.txt"

; MUI Start Menu Folder Page Configuration
!define MUI_STARTMENUPAGE_DEFAULTFOLDER "${SHOES_NAME}"
!define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKLM" 
!define MUI_STARTMENUPAGE_REGISTRY_KEY "${SHOES_INST_KEY}" 
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"

!insertmacro MUI_PAGE_STARTMENU Application $StartMenuFolder

!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!define MUI_FINISHPAGE_NOREBOOTSUPPORT
!define MUI_FINISHPAGE_TITLE_3LINES
!define MUI_FINISHPAGE_RUN "$INSTDIR\${SHOES_NAME}.exe"
!insertmacro MUI_PAGE_FINISH

; MUI uninstaller pages
!insertmacro MUI_UNPAGE_DIRECTORY
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

!insertmacro MUI_LANGUAGE "English"

; NSIS Output information
Name "${SHOES_NAME} ${SHOES_VERSION}"
OutFile "${SHOES_NAME}-${SHOES_VERSION}.exe"
InstallDir "$PROGRAMFILES\${SHOES_NAME}"
InstallDirRegKey HKLM "${SHOES_INST_KEY}" ""
RequestExecutionLevel admin

ShowInstDetails show
ShowUnInstDetails show

Section "MainSection" SEC01
   SetOutPath "$INSTDIR"
   SetOverwrite ifnewer

   !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
      CreateDirectory "$SMPROGRAMS\${SHOES_NAME}"
      CreateShortCut "$SMPROGRAMS\${SHOES_NAME}\${SHOES_NAME}.lnk" "$INSTDIR\${SHOES_NAME}.exe"
      CreateShortCut "$SMPROGRAMS\${SHOES_NAME}\Manual.lnk" "$INSTDIR\${SHOES_NAME}.exe" "--manual"
      CreateShortCut "$SMPROGRAMS\${SHOES_NAME}\Packager.lnk" "$INSTDIR\${SHOES_NAME}.exe" "--package"
   !insertmacro MUI_STARTMENU_WRITE_END
   
   CreateShortCut "$DESKTOP\${SHOES_NAME}.lnk" "$INSTDIR\${SHOES_NAME}.exe"
   
   File /r /x nsis ..\*.*
   
   ${EnvVarUpdate} $0 "PATH" "A" HKLM $INSTDIR
   ${EnvVarUpdate} $0 "FONTCONFIG_FILE" "A" HKLM "$INSTDIR\etc\fonts\fonts.conf"
   ${registerExtension} "$INSTDIR\${SHOES_NAME}.exe" ".shy" "Shoes Application"
   DetailPrint "Building font cache, this may take a while..."
   ExecWait '"$INSTDIR\fc-cache.exe" "-f"'
SectionEnd

Section -AdditionalIcons
   CreateShortCut "$SMPROGRAMS\${SHOES_NAME}\Uninstall.lnk" "$INSTDIR\uninst.exe"
SectionEnd

Section -Post
   WriteUninstaller "$INSTDIR\uninst.exe"
   WriteRegStr HKLM "${SHOES_INST_KEY}" "" "$INSTDIR\${SHOES_NAME}.exe"
   ;WriteRegStr HKLM "${SHOES_INST_KEY}" "base" "$INSTDIR"
   WriteRegStr ${SHOES_UNINST_ROOT_KEY} "${SHOES_UNINST_KEY}" "DisplayName" "$(^Name)"
   WriteRegStr ${SHOES_UNINST_ROOT_KEY} "${SHOES_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
   WriteRegStr ${SHOES_UNINST_ROOT_KEY} "${SHOES_UNINST_KEY}" "DisplayIcon" "$INSTDIR\${SHOES_NAME}.exe"
   WriteRegStr ${SHOES_UNINST_ROOT_KEY} "${SHOES_UNINST_KEY}" "DisplayVersion" "${SHOES_VERSION}"
   WriteRegStr ${SHOES_UNINST_ROOT_KEY} "${SHOES_UNINST_KEY}" "URLInfoAbout" "${SHOES_WEB_SITE}"
   WriteRegStr ${SHOES_UNINST_ROOT_KEY} "${SHOES_UNINST_KEY}" "Publisher" "${SHOES_PUBLISHER}"
  
   ${GetSize} "$INSTDIR" "/S=0K" $0 $1 $2
   IntFmt $0 "0x%08X" $0
   WriteRegDWORD HKLM "${SHOES_UNINST_KEY}" "EstimatedSize" "$0"
SectionEnd

Section Uninstall
   !insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuFolder

   Delete "$DESKTOP\${SHOES_NAME}.lnk"
   Delete "$SMPROGRAMS\${SHOES_NAME}\Manual.lnk"
   Delete "$SMPROGRAMS\${SHOES_NAME}\Packager.lnk"
   Delete "$SMPROGRAMS\${SHOES_NAME}\${SHOES_NAME}.lnk"
   Delete "$SMPROGRAMS\${SHOES_NAME}\Uninstall.lnk"

   RMDir "$SMPROGRAMS\${SHOES_NAME}"
   RMDir /r "$INSTDIR\*.*"
   RMDir "$INSTDIR"
   
   ${unregisterExtension} ".shy" "Shoes Application"
   ${un.EnvVarUpdate} $0 "PATH" "R" HKLM $INSTDIR
   ${un.EnvVarUpdate} $0 "FONTCONFIG_FILE" "R" HKLM "$INSTDIR\etc\fonts\fonts.conf"

   DeleteRegKey ${SHOES_UNINST_ROOT_KEY} "${SHOES_UNINST_KEY}"
   DeleteRegKey HKLM "${SHOES_INST_KEY}"
   SetAutoClose true
   StrCpy $UninstallerHasFinished true
SectionEnd

Function un.onInit
   MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "Uninstall $(^Name)?" IDYES +2
FunctionEnd

Function un.onUninstSuccess
   ${If} $UninstallerHasFinished == true
      HideWindow
      MessageBox MB_ICONINFORMATION|MB_OK "$(^Name) has been uninstalled."
   ${EndIf}
FunctionEnd
