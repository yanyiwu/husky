CURL_RES=curl.res
TMP=curl.res.tmp
curl -s "http://127.0.0.1:11257" >> $TMP
if diff $TMP $CURL_RES >> /dev/null
then
    echo "ok";
else
    echo "failed."
fi

rm $TMP
