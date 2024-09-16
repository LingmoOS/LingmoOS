; requires Windows 10, Windows 7 Service Pack 1, Windows 8, Windows 8.1, Windows Server 2003 Service Pack 2, Windows Server 2008 R2 SP1, Windows Server 2008 Service Pack 2, Windows Server 2012, Windows Vista Service Pack 2, Windows XP Service Pack 3
; http://www.visualstudio.com/en-us/downloads/

[CustomMessages]
vcredist_title=Visual C++ 2015-2022 Redistributable
vcredist_title_x64=Visual C++ 2015-2022 64-Bit Redistributable

vcredist_size=13.2 MB
vcredist_size_x64=24.2 MB

[Code]
const
	vcredist_url = 'http://download.visualstudio.microsoft.com/download/pr/a061be25-c14a-489a-8c7c-bb72adfb3cab/C61CEF97487536E766130FA8714DD1B4143F6738BFB71806018EEE1B5FE6F057/VC_redist.x86.exe';
	vcredist_url_x64 = 'http://download.visualstudio.microsoft.com/download/pr/a061be25-c14a-489a-8c7c-bb72adfb3cab/4DFE83C91124CD542F4222FE2C396CABEAC617BB6F59BDCBDF89FD6F0DF0A32F/VC_redist.x64.exe';

	vcredist_upgradecode = '{65E5BD06-6392-3027-8C26-853107D3CF1A}';
	vcredist_upgradecode_x64 = '{36F68A90-239C-34DF-B58C-64B30153CE35}';

procedure vcredist201522(minVersion: string);
begin
	if (not IsIA64()) then begin
		if (not msiproductupgrade(GetString(vcredist_upgradecode, vcredist_upgradecode_x64, ''), minVersion)) then
			AddProduct('vc_redist.x64.exe',
				'/passive /norestart',
				CustomMessage('vcredist_title' + GetArchitectureString()),
				CustomMessage('vcredist_size' + GetArchitectureString()),
				GetString(vcredist_url, vcredist_url_x64, ''),
				false, false, false);
	end;
end;

[Setup]
