gaugesDir = 'gauges';
outdir = 'outdir';
samplesCount = 200;

fileNames = dir(gaugesDir);
mkdir(outdir);
for fileName = fileNames'
    if (strcmp(fileName.name, '..') || strcmp(fileName.name, '.'))
        continue;
    end;
    [tok rem] = strtok(fileName.name, '.');
    if (~strcmp(rem, '.bmp') && ~strcmp(rem, '.BMP') && ~strcmp(rem, '.jpg') & ~strcmp(rem, '.JPG'))
        continue;
    end;
    gauge = imread(strcat(gaugesDir, '/', fileName.name));
    gauge = gauge * (256 / max(max(gauge)));
    gaugeName = strtok(fileName.name, '.');
    gaugeFolder = strcat(outdir, '/', gaugeName);
    generateAndSave(gauge, gaugeFolder, samplesCount);
    disp(gaugeFolder);
    
end