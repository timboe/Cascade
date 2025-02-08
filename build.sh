rm -r Cascada.pdx

awk -F '=' '/buildNumber/{$2=$2+1}1' OFS='=' Source/pdxinfo > tmp
mv tmp Source/pdxinfo

cd ${PLAYDATE_SDK_PATH}/Disk/Data/uk.co.tim-martin.cascada/
for f in *.bmp; do convert $f -resize 50% -dither FloydSteinberg -monochrome "`basename $f .bmp`.png"; done
rm *bmp
cd -
rm Source/images/holes/*png
cp ${PLAYDATE_SDK_PATH}/Disk/Data/uk.co.tim-martin.cascada/*png Source/images/holes

make clean
make

zip -r -q Cascada_v0.1.pdx.zip Cascada.pdx



