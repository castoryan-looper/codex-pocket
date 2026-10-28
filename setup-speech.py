"""Download public model weights once; audio is never sent to the download host."""
from pathlib import Path
from huggingface_hub import snapshot_download

if __name__ == '__main__':
    path = Path(__file__).resolve().parent/'models/whisper-small'
    snapshot_download('Systran/faster-whisper-small',local_dir=path,
        allow_patterns=['config.json','model.bin','tokenizer.json','vocabulary.*','preprocessor_config.json'])
    print('Offline speech model ready:',path)
