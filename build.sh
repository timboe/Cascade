rm -r Cascade.pdx

awk -F '=' '/buildNumber/{$2=$2+1}1' OFS='=' Source/pdxinfo > tmp
mv tmp Source/pdxinfo

cd ${PLAYDATE_SDK_PATH}/Disk/Data/uk.co.tim-martin.cascade/
for f in *.bmp; do convert $f -resize 50% -dither FloydSteinberg -monochrome "`basename $f .bmp`.png"; done
cd -
cp -v ${PLAYDATE_SDK_PATH}/Disk/Data/uk.co.tim-martin.cascade/*png Source/images/holes

make

zip -r -q Cascade_v0.1.pdx.zip Cascade.pdx



