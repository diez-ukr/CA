function generateAndSave(srcImage, folderName, count)
    mkdir(folderName);
    for k = 1 : count
        outImage = srcImage;
        distortionCount = randi(10);
        for i = 1 : distortionCount
            switch(randi(4))
                case 1
                    outImage = imresize(outImage, (70 + randi(30)) / 100);
                case 2
                    outImage = 256 - outImage;
                    outImage = imrotate(outImage, randi(360), 'nearest', 'crop');
                    outImage = 256 - outImage;
                case 3
                    G = fspecial('gaussian',[randi(5) randi(5)], randi(10));
                    outImage = imfilter(outImage, G, 'same');
                case 4
                    outImage = imnoise(outImage, 'salt & pepper', randi(10)/200);
            end
        end
        outImage = imresize(outImage, [200, 200]);
        fileName = strcat(folderName, '/', num2str(k), '.bmp');
        imwrite(outImage, fileName);
    end
end