/**************************************************************************
**
** Copyright (C) 2017 shoesrb.com
** Contact: http://www.shoesrb.com/
**
** This file is part of Shoes.

**************************************************************************/

function Component() {
   var programFiles = installer.environmentVariable("ProgramFiles");
   if (programFiles != "")
      installer.setValue("TargetDir", programFiles + "/@Name@");
   
   installer.setDefaultPageVisible(QInstaller.Introduction, false);
   installer.setDefaultPageVisible(QInstaller.TargetDirectory, true);
   installer.setDefaultPageVisible(QInstaller.ComponentSelection, false);
   installer.setDefaultPageVisible(QInstaller.ReadyForInstallation, true);
   installer.setDefaultPageVisible(QInstaller.StartMenuSelection, true);
   installer.setDefaultPageVisible(QInstaller.LicenseCheck, false);
}

Component.prototype.isDefault = function() {
   return true;
}

Component.prototype.createOperations = function() {
    try {
      component.createOperations();
      if (installer.value("os") === "win") {
         component.addOperation("RegisterFileType", "shy", "@TargetDir@\\@Name@.exe '%1'", "Shoes application", "text/plain", "@TargetDir@\\@Name@.exe,0");
         component.addOperation("CreateShortcut", "@TargetDir@\\@Name@.exe", "@StartMenuDir@\\@Name@.lnk", "workingDirectory=@TargetDir@", "iconPath=@TargetDir@\\@Name@.exe", "iconId=0");
         component.addOperation("CreateShortcut", "@TargetDir@\\@Name@.exe", "@StartMenuDir@\\Manual.lnk", "--manual", "workingDirectory=@TargetDir@", "iconPath=@TargetDir@\\@Name@.exe", "iconId=0");
         component.addOperation("CreateShortcut", "@TargetDir@\\@Name@.exe", "@StartMenuDir@\\Packager.lnk", "--package", "workingDirectory=@TargetDir@", "iconPath=@TargetDir@\\@Name@.exe", "iconId=0");
         component.addOperation("CreateShortcut", "@TargetDir@\\@Name@.exe", "@DesktopDir@\\@Name@.lnk", "workingDirectory=@TargetDir@", "iconPath=@TargetDir@\\@Name@.exe", "iconId=0");

         var cmd = installer.environmentVariable("SystemRoot") + "\\System32\\cmd.exe";
         var reg = installer.environmentVariable("SystemRoot") + "\\System32\\reg.exe";
         
         var content = installer.execute(cmd, new Array("/C", reg,  "QUERY", "HKEY_CURRENT_USER\\Environment", "/v", "PATH"))[0].replace(/\s+/g,' ').trim().split(" ");
         var path = content[content.length - 1];
         component.addOperation("GlobalConfig", "HKEY_CURRENT_USER\\Environment", "PATH", path + ";" + installer.value("TargetDir").replace(/\//g, '\\'));
      }
   } catch (e) {
      console.log(e);
   }
}

