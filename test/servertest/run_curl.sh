CURL_RES=curl.res
TMP=curl.res.tmp
for i in $(seq 10)
do
    echo $i
    curl -s "http://127.0.0.1:11257" >> $TMP
done
if diff $TMP $CURL_RES >> /dev/null
then
    echo "ok";
else
    echo "failed."
fi

rm $TMP
