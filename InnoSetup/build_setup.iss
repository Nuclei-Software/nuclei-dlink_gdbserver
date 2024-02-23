#define MyAppName "dlink-gdbserver"
#define MyAppVersion "#VERSION#"
#define MyAppVersionInfoVersion "#VERSIONINFOVERSION#"
#define MyAppPublisher "wangyanwen@nucleisys.com"
#define MyAppURL "https://github.com/Nuclei-Software/nuclei-dlink_gdbserver"
#define MyAppExeName "dlink_gdbserver.exe"
#define MyAppOutputName "dlink_gdbserver_v#VERSION#_windows_setup"

[Setup]
AppId={{FB978856-5DFB-4B2D-8DDB-B4DBB585715B}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
;AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
OutputBaseFilename={#MyAppOutputName}
Compression=lzma
SolidCompression=yes

VersionInfoVersion={#MyAppVersionInfoVersion}
UninstallDisplayIcon={app}/{#MyAppExeName}
InfoBeforeFile=..\InnoSetup\Info.txt
OutPutdir=..\InnoSetup
SetupIconFile=..\logo.ico
PrivilegesRequired=admin
AllowNoIcons=no
DisableProgramGroupPage=yes
WizardStyle=modern

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "japanese"; MessagesFile: "compiler:Languages\Japanese.isl"
Name: "chinese"; MessagesFile: ".\ChineseSimplified.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked; OnlyBelowVersion: 6.1; Check: not IsAdminInstallMode

[Files]
Source: "..\InnoSetup\build\dlink_gdbserver.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\InnoSetup\build\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{group}\{#MyAppName}-mini"; Filename: "{app}\{#MyAppExeName}"; Parameters: "-m true"
Name: "{group}\{cm:ProgramOnTheWeb,{#MyAppName}}"; Filename: "{#MyAppURL}"
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: quicklaunchicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

[Registry]
Root: HKCR; Subkey: "Directory\shell\dlink_gdbserver"; ValueType: string; ValueName: ""; ValueData: "Open with dlink_gdbserver"; Flags: uninsdeletekey ;
Root: HKCR; Subkey: "Directory\shell\dlink_gdbserver"; ValueType: string; ValueName: "Icon"; ValueData: "{app}\{#MyAppExeName},0"; Flags: uninsdeletekey
Root: HKCR; Subkey: "Directory\shell\dlink_gdbserver\command"; ValueType: string; ValueName: ""; ValueData: """{app}\{#MyAppExeName}"" ""-m"" ""true"" ""-s"" ""%1"""; Flags: uninsdeletekey
Root: HKCR; Subkey: "Directory\Background\shell\dlink_gdbserver"; ValueType: string; ValueName: ""; ValueData: "Open with dlink_gdbserver"; Flags: uninsdeletekey
Root: HKCR; Subkey: "Directory\Background\shell\dlink_gdbserver"; ValueType: string; ValueName: "Icon"; ValueData: "{app}\{#MyAppExeName},0"; Flags: uninsdeletekey
Root: HKCR; Subkey: "Directory\Background\shell\dlink_gdbserver\command"; ValueType: string; ValueName: ""; ValueData: """{app}\{#MyAppExeName}"" ""-m"" ""true"" ""-s"" ""%V"""; Flags: uninsdeletekey
