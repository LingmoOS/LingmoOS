; requires Windows 10, Windows 7 Service Pack 1, Windows 8, Windows 8.1, Windows Server 2003 Service Pack 2, Windows Server 2008 R2 SP1, Windows Server 2008 Service Pack 2, Windows Server 2012, Windows Vista Service Pack 2, Windows XP Service Pack 3
; http://www.visualstudio.com/en-us/downloads/

[CustomMessages]
bonjour_title=Bonjour
bonjour_title_x64=Bonjour

bonjour_size=2.25 MB
bonjour_size_x64=2.56 MB

;http://download.info.apple.com/Mac_OS_X/061-8098.20100603.gthyu/BonjourPSSetup.exe
[Code]
const
	bonjour_url = 'http://binaries.symless.com/bonjour/Bonjour.msi';
	bonjour_url_x64 = 'http://binaries.symless.com/bonjour/Bonjour64.msi';

	bonjour_upgradecode = '{B91110FB-33B4-468B-90C2-4D5E8AE3FAE1}';
	bonjour_upgradecode_x64 = '{46AE3251-43D6-41CF-8CDF-E902C38516D1}';

procedure bonjour(minVersion: string);
begin
	if (not IsIA64()) then begin
		if (not msiproductupgrade(GetString(bonjour_upgradecode, bonjour_upgradecode_x64, ''), minVersion)) then
			AddProduct('Bonjour64.msi',
				'/passive /norestart',
				CustomMessage('bonjour_title' + GetArchitectureString()),
				CustomMessage('bonjour_size' + GetArchitectureString()),
				GetString(bonjour_url, bonjour_url_x64, ''),
				false, false, false);
	end;
end;

[Setup]
