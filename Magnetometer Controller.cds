<distribution version="9.0.0" name="Magnetometer Controller" type="MSI">
	<prebuild>
		<workingdir>workspacedir</workingdir>
		<actions></actions></prebuild>
	<postbuild>
		<workingdir>workspacedir</workingdir>
		<actions></actions></postbuild>
	<msi GUID="{4D1E7617-5BAA-4BEC-A5A3-2472C4C067FE}">
		<general appName="Magnetometer Controller" outputLocation="c:\Documents and Settings\OmegaUser\My Documents\Omega Project\Programs\Magnetometer" relOutputLocation="..\Magnetometer" outputLocationWithVars="c:\Documents and Settings\OmegaUser\My Documents\Omega Project\Programs\Magnetometer" relOutputLocationWithVars="..\Magnetometer" autoIncrement="false" version="1.0.0">
			<arp company="Pines Lab" companyURL="" supportURL="" contact="" phone="" comments=""/>
			<summary title="Magnetometer Controller Installer" subject="" keyWords="" comments="" author="Paul Ganssle"/></general>
		<userinterface language="English" readMe="" license="">
			<dlgstrings welcomeTitle="Magnetometer Controller" welcomeText=""/></userinterface>
		<dirs appDirID="103">
			<installDir name="Programs" dirID="100" parentID="103" isMSIDir="false" visible="true"/>
			<installDir name="Experiments" dirID="101" parentID="103" isMSIDir="false" visible="true"/>
			<installDir name="Magnetometer Controller" dirID="102" parentID="7" isMSIDir="false" visible="true"/>
			<installDir name="[Program Files]" dirID="2" parentID="-1" isMSIDir="true" visible="true"/>
			<installDir name="Magnetometer Controller" dirID="103" parentID="2" isMSIDir="false" visible="true"/>
			<installDir name="[Start&gt;&gt;Programs]" dirID="7" parentID="-1" isMSIDir="true" visible="true"/></dirs>
		<files>
			<simpleFile fileID="0" sourcePath="c:\Documents and Settings\OmegaUser\My Documents\Omega Project\Programs\Magnetometer Controller\MCTypedef.h" relSourcePath="MCTypedef.h" relSourceBase="0" targetDir="103" readonly="false" hidden="false" system="false" regActiveX="false" runAfterInstallStyle="IMMEDIATELY_RESUME_INSTALL" cmdLineArgs="" runAfterInstall="false" uninstCmdLnArgs="" runUninst="false"/>
			<simpleFile fileID="1" sourcePath="c:\Documents and Settings\OmegaUser\My Documents\Omega Project\Programs\Magnetometer Controller\Defaults.txt" relSourcePath="Defaults.txt" relSourceBase="0" targetDir="103" readonly="false" hidden="true" system="false" regActiveX="false" runAfterInstallStyle="IMMEDIATELY_RESUME_INSTALL" cmdLineArgs="" runAfterInstall="false" uninstCmdLnArgs="" runUninst="false"/>
			<simpleFile fileID="2" sourcePath="c:\Documents and Settings\OmegaUser\My Documents\Omega Project\Programs\Magnetometer Controller\FFTW\libfftw3l-3.lib" relSourcePath="FFTW\libfftw3l-3.lib" relSourceBase="0" targetDir="103" readonly="false" hidden="false" system="false" regActiveX="false" runAfterInstallStyle="IMMEDIATELY_RESUME_INSTALL" cmdLineArgs="" runAfterInstall="false" uninstCmdLnArgs="" runUninst="false"/>
			<simpleFile fileID="3" sourcePath="c:\Documents and Settings\OmegaUser\My Documents\Omega Project\Programs\Magnetometer Controller\FFTW\fftw3.f" relSourcePath="FFTW\fftw3.f" relSourceBase="0" targetDir="103" readonly="false" hidden="false" system="false" regActiveX="false" runAfterInstallStyle="IMMEDIATELY_RESUME_INSTALL" cmdLineArgs="" runAfterInstall="false" uninstCmdLnArgs="" runUninst="false"/>
			<simpleFile fileID="4" sourcePath="c:\Documents and Settings\OmegaUser\My Documents\Omega Project\Programs\Magnetometer Controller\Magnetometer Controller.c" relSourcePath="Magnetometer Controller.c" relSourceBase="0" targetDir="103" readonly="false" hidden="false" system="false" regActiveX="false" runAfterInstallStyle="IMMEDIATELY_RESUME_INSTALL" cmdLineArgs="" runAfterInstall="false" uninstCmdLnArgs="" runUninst="false"/>
			<simpleFile fileID="5" sourcePath="c:\Documents and Settings\OmegaUser\My Documents\Omega Project\Programs\Magnetometer Controller\FFTW\fftw3.h" relSourcePath="FFTW\fftw3.h" relSourceBase="0" targetDir="103" readonly="false" hidden="false" system="false" regActiveX="false" runAfterInstallStyle="IMMEDIATELY_RESUME_INSTALL" cmdLineArgs="" runAfterInstall="false" uninstCmdLnArgs="" runUninst="false"/>
			<simpleFile fileID="6" sourcePath="c:\SpinCore\SpinAPI\dll\spinapi.h" relSourceBase="0" targetDir="103" readonly="false" hidden="false" system="false" regActiveX="false" runAfterInstallStyle="IMMEDIATELY_RESUME_INSTALL" cmdLineArgs="" runAfterInstall="false" uninstCmdLnArgs="" runUninst="false"/>
			<simpleFile fileID="7" sourcePath="c:\Documents and Settings\OmegaUser\My Documents\Omega Project\Programs\Magnetometer Controller\FFTW\libfftw3l-3.def" relSourcePath="FFTW\libfftw3l-3.def" relSourceBase="0" targetDir="103" readonly="false" hidden="false" system="false" regActiveX="false" runAfterInstallStyle="IMMEDIATELY_RESUME_INSTALL" cmdLineArgs="" runAfterInstall="false" uninstCmdLnArgs="" runUninst="false"/>
			<simpleFile fileID="8" sourcePath="c:\Documents and Settings\OmegaUser\My Documents\Omega Project\Programs\Magnetometer Controller\FFTW\libfftw3-3.def" relSourcePath="FFTW\libfftw3-3.def" relSourceBase="0" targetDir="103" readonly="false" hidden="false" system="false" regActiveX="false" runAfterInstallStyle="IMMEDIATELY_RESUME_INSTALL" cmdLineArgs="" runAfterInstall="false" uninstCmdLnArgs="" runUninst="false"/>
			<simpleFile fileID="9" sourcePath="c:\Documents and Settings\OmegaUser\My Documents\Omega Project\Programs\Magnetometer Controller\FFTW\libfftw3f-3.lib" relSourcePath="FFTW\libfftw3f-3.lib" relSourceBase="0" targetDir="103" readonly="false" hidden="false" system="false" regActiveX="false" runAfterInstallStyle="IMMEDIATELY_RESUME_INSTALL" cmdLineArgs="" runAfterInstall="false" uninstCmdLnArgs="" runUninst="false"/>
			<simpleFile fileID="10" sourcePath="c:\Documents and Settings\OmegaUser\My Documents\Omega Project\Programs\Magnetometer Controller\Magnetometer Controller.exe" targetDir="103" readonly="false" hidden="false" system="false" regActiveX="false" runAfterInstallStyle="IMMEDIATELY_RESUME_INSTALL" cmdLineArgs="" runAfterInstall="false" uninstCmdLnArgs="" runUninst="false"/>
			<simpleFile fileID="11" sourcePath="c:\Documents and Settings\OmegaUser\My Documents\Omega Project\Programs\Magnetometer Controller\libfftw3-3.dll" targetDir="103" readonly="false" hidden="false" system="false" regActiveX="false" runAfterInstallStyle="IMMEDIATELY_RESUME_INSTALL" cmdLineArgs="" runAfterInstall="false" uninstCmdLnArgs="" runUninst="false"/>
			<simpleFile fileID="12" sourcePath="C:\WINDOWS\system32\spinapi.dll" targetDir="103" readonly="false" hidden="false" system="false" regActiveX="false" runAfterInstallStyle="IMMEDIATELY_RESUME_INSTALL" cmdLineArgs="" runAfterInstall="false" uninstCmdLnArgs="" runUninst="false"/>
			<simpleFile fileID="13" sourcePath="c:\Documents and Settings\OmegaUser\My Documents\Omega Project\Programs\Magnetometer Controller\FFTW\libfftw3f-3.def" relSourcePath="FFTW\libfftw3f-3.def" relSourceBase="0" targetDir="103" readonly="false" hidden="false" system="false" regActiveX="false" runAfterInstallStyle="IMMEDIATELY_RESUME_INSTALL" cmdLineArgs="" runAfterInstall="false" uninstCmdLnArgs="" runUninst="false"/>
			<simpleFile fileID="14" sourcePath="c:\Documents and Settings\OmegaUser\My Documents\Omega Project\Programs\Magnetometer Controller\SavedSessionBuffer.txt" relSourcePath="SavedSessionBuffer.txt" relSourceBase="0" targetDir="103" readonly="false" hidden="true" system="false" regActiveX="false" runAfterInstallStyle="IMMEDIATELY_RESUME_INSTALL" cmdLineArgs="" runAfterInstall="false" uninstCmdLnArgs="" runUninst="false"/>
			<simpleFile fileID="15" sourcePath="c:\Documents and Settings\OmegaUser\My Documents\Omega Project\Programs\Magnetometer Controller\FFTW\libfftw3-3.exp" relSourcePath="FFTW\libfftw3-3.exp" relSourceBase="0" targetDir="103" readonly="false" hidden="false" system="false" regActiveX="false" runAfterInstallStyle="IMMEDIATELY_RESUME_INSTALL" cmdLineArgs="" runAfterInstall="false" uninstCmdLnArgs="" runUninst="false"/>
			<simpleFile fileID="16" sourcePath="c:\Documents and Settings\OmegaUser\My Documents\Omega Project\Programs\Magnetometer Controller\Magnetometer Controller.h" relSourcePath="Magnetometer Controller.h" relSourceBase="0" targetDir="103" readonly="false" hidden="false" system="false" regActiveX="false" runAfterInstallStyle="IMMEDIATELY_RESUME_INSTALL" cmdLineArgs="" runAfterInstall="false" uninstCmdLnArgs="" runUninst="false"/>
			<simpleFile fileID="17" sourcePath="c:\Documents and Settings\OmegaUser\My Documents\Omega Project\Programs\Magnetometer Controller\FFTW\libfftw3-3.lib" relSourcePath="FFTW\libfftw3-3.lib" relSourceBase="0" targetDir="103" readonly="false" hidden="false" system="false" regActiveX="false" runAfterInstallStyle="IMMEDIATELY_RESUME_INSTALL" cmdLineArgs="" runAfterInstall="false" uninstCmdLnArgs="" runUninst="false"/>
			<simpleFile fileID="18" sourcePath="c:\Documents and Settings\OmegaUser\My Documents\Omega Project\Programs\Magnetometer Controller\LastProgramBuffer.txt" relSourcePath="LastProgramBuffer.txt" relSourceBase="0" targetDir="103" readonly="false" hidden="true" system="false" regActiveX="false" runAfterInstallStyle="IMMEDIATELY_RESUME_INSTALL" cmdLineArgs="" runAfterInstall="false" uninstCmdLnArgs="" runUninst="false"/>
			<simpleFile fileID="19" sourcePath="c:\Documents and Settings\OmegaUser\My Documents\Omega Project\Programs\Magnetometer Controller\MC.h" relSourcePath="MC.h" relSourceBase="0" targetDir="103" readonly="false" hidden="false" system="false" regActiveX="false" runAfterInstallStyle="IMMEDIATELY_RESUME_INSTALL" cmdLineArgs="" runAfterInstall="false" uninstCmdLnArgs="" runUninst="false"/>
			<simpleFile fileID="20" sourcePath="c:\SpinCore\SpinAPI\dll\spinapi.lib" relSourceBase="0" targetDir="103" readonly="false" hidden="false" system="false" regActiveX="false" runAfterInstallStyle="IMMEDIATELY_RESUME_INSTALL" cmdLineArgs="" runAfterInstall="false" uninstCmdLnArgs="" runUninst="false"/>
			<simpleFile fileID="21" sourcePath="c:\Documents and Settings\OmegaUser\My Documents\Omega Project\Programs\Magnetometer Controller\Magnetometer Controller.exe" targetDir="102" readonly="false" hidden="false" system="false" regActiveX="false" runAfterInstallStyle="IMMEDIATELY_RESUME_INSTALL" cmdLineArgs="" runAfterInstall="false" uninstCmdLnArgs="" runUninst="false"/>
			<simpleFile fileID="22" sourcePath="c:\Documents and Settings\OmegaUser\My Documents\Omega Project\Programs\Magnetometer Controller\FFTW\libfftw3f-3.exp" relSourcePath="FFTW\libfftw3f-3.exp" relSourceBase="0" targetDir="103" readonly="false" hidden="false" system="false" regActiveX="false" runAfterInstallStyle="IMMEDIATELY_RESUME_INSTALL" cmdLineArgs="" runAfterInstall="false" uninstCmdLnArgs="" runUninst="false"/>
			<simpleFile fileID="23" sourcePath="c:\Documents and Settings\OmegaUser\My Documents\Omega Project\Programs\Magnetometer Controller\FFTW\libfftw3l-3.exp" relSourcePath="FFTW\libfftw3l-3.exp" relSourceBase="0" targetDir="103" readonly="false" hidden="false" system="false" regActiveX="false" runAfterInstallStyle="IMMEDIATELY_RESUME_INSTALL" cmdLineArgs="" runAfterInstall="false" uninstCmdLnArgs="" runUninst="false"/>
			<simpleFile fileID="24" sourcePath="c:\Documents and Settings\OmegaUser\My Documents\Omega Project\Programs\Magnetometer Controller\Magnetometer Controller.uir" relSourcePath="Magnetometer Controller.uir" relSourceBase="0" targetDir="103" readonly="false" hidden="false" system="false" regActiveX="false" runAfterInstallStyle="IMMEDIATELY_RESUME_INSTALL" cmdLineArgs="" runAfterInstall="false" uninstCmdLnArgs="" runUninst="false"/></files>
		<fileGroups>
			<projectOutput targetType="0" dirID="103" projectID="0">
				<fileID>10</fileID></projectOutput>
			<projectOutput targetType="0" dirID="102" projectID="0">
				<fileID>21</fileID></projectOutput>
			<projectDependencies dirID="103" projectID="0">
				<fileID>11</fileID>
				<fileID>12</fileID></projectDependencies></fileGroups>
		<shortcuts>
			<shortcut name="Magnetometer Controller" targetFileID="10" destDirID="102" cmdLineArgs="" description="" runStyle="NORMAL"/></shortcuts>
		<mergemodules/>
		<products>
			<product name="NI LabVIEW Run-Time Engine 8.2.1" productID="{45FA54F6-8574-49D2-9E2D-0BDDE6237822}" UC="{B5171839-26E3-48D9-9FD6-AF7F39055146}" path="C:\Program Files\National Instruments\Shared\ProductCache\NI LabVIEW Rutime Engine [8.2.1]\" flavorID="DefaultFull" flavorName="Full" verRestr="false" coreVer="8.2.379"/>
			<product name="NI-488.2 2.6" productID="{937614F5-ABD1-4068-92AB-4CA4AA1010D2}" UC="{357F6618-C660-41A2-A185-5578CC876D1D}" path="C:\National Instruments Downloads\NI-488.2\2.6\" flavorID="_Deployable_" flavorName="NI-488.2" verRestr="false" coreVer="2.60.49153"/></products>
		<nonAutoSelectProducts>
			<productID>{91239260-0A14-451C-9E50-7CB74204ABE1}</productID>
			<productID>{937614F5-ABD1-4068-92AB-4CA4AA1010D2}</productID></nonAutoSelectProducts>
		<runtimeEngine cvirte="true" instrsup="true" lvrt="false" analysis="true" netvarsup="true" dotnetsup="false" activeXsup="false" lowlevelsup="false" rtutilsup="false" installToAppDir="false"/>
		<advanced mediaSize="10000">
			<launchConditions>
				<condition>MINOS_WIN2K_SP3</condition>
			</launchConditions>
			<hardwareConfig></hardwareConfig>
			<includeConfigProducts>true</includeConfigProducts>
			<maxImportVisible>silent</maxImportVisible>
			<maxImportMode>merge</maxImportMode></advanced>
		<Projects NumProjects="1">
			<Project000 ProjectID="0" ProjectAbsolutePath="c:\Documents and Settings\OmegaUser\My Documents\Omega Project\Programs\Magnetometer Controller\Magnetometer Controller.prj" ProjectRelativePath="Magnetometer Controller.prj"/></Projects>
		<buildData progressBarRate="0.002360082508484">
			<progressTimes>
					<Begin>0.000000000000000</Begin>
					<ProductsAdded>1.264999866485596</ProductsAdded>
					<DPConfigured>1.750000000000000</DPConfigured>
					<DPMergeModulesAdded>21.764999866485596</DPMergeModulesAdded>
					<DPClosed>25.062000274658203</DPClosed>
					<DistributionsCopied>29.590000000000000</DistributionsCopied>
					<End>433.286000274658140</End></progressTimes></buildData>
	</msi>
</distribution>
