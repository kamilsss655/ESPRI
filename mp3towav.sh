# script to convert all mp3 files to 32kHz wav files
# to run: ./mp3towav.sh *.mp3
mp3towav() {
    # create out dir
    mkdir out;
    [[ $# -eq 0 ]] && { echo "mp3towav mp3file"; exit 1; }
    for i in "$@"; do
        # create .wav file name
        local out="${i%.*}.wav"
        [[ -f "$i" ]] && { 
            echo -n "Processing ${i}...";
            mpg123 -w "${out}" "$i" &>/dev/null && echo "done." || echo "failed.";
        }
        [[ -f "$i" ]] && { 
            echo -n "Resampling ${out}...";
            sox "${out}" -r 32k -e signed -b 16 -c 1 "out/${out}"  &>/dev/null && echo "done." || echo "failed.";
        }
    done
}

mp3towav *.mp3