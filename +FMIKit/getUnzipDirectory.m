function unzipdir = getUnzipDirectory(block)
% Internal API - do not use

unzipdir = '';

userData = getUserData(block);

if ~isempty(userData)
    unzipdir = userData.unzipDirectory;

    mdlfile = get_param(bdroot(block), 'FileName');
    mdldir = fileparts(mdlfile);
    mdlreldir = fullfile(mdldir, unzipdir);

    if isdir(mdlreldir)
        unzipdir = mdlreldir;
    end
end

end
