function rtwsfcnfmi_make_rtw_hook(hookMethod, modelName, rtwRoot, templateMakefile, buildOpts, buildArgs, buildInfo)

switch hookMethod
    case 'after_make'
        disp('### Creating FMU archive')
        zip('FMUArchive.zip', '*', 'FMUArchive')
        copyfile('FMUArchive.zip', ['../' modelName '_sf.fmu'])
end

end
