rm -r Cascade.pdx

awk -F '=' '/buildNumber/{$2=$2+1}1' OFS='=' Source/pdxinfo > tmp
mv tmp Source/pdxinfo

make

zip -r -q Cascade_v0.1.pdx.zip Cascade.pdx



