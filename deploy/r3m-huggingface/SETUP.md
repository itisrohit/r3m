# Setup Hugging Face Spaces Deployment

## Step 1: Get Hugging Face Token

1. Go to [Hugging Face Settings](https://huggingface.co/settings/tokens)
2. Click "New token"
3. Give it a name like "R3M API Deployment"
4. Select "Write" permissions
5. Copy the token

## Step 2: Add Token to GitHub Secrets

1. Go to your GitHub repository
2. Click "Settings" → "Secrets and variables" → "Actions"
3. Click "New repository secret"
4. Name: `HF_TOKEN`
5. Value: Paste your Hugging Face token

## Step 3: Deploy

1. Push to `testings2` branch
2. The workflow will automatically:
   - Create a new Hugging Face Space
   - Build your C++ Docker container
   - Deploy it to `https://your-username-r3m-api.hf.space`

## Step 4: Test Your API

Once deployed, test your endpoints:

```bash
# Health check
curl https://your-username-r3m-api.hf.space/health

# API info
curl https://your-username-r3m-api.hf.space/info

# Process document
curl -X POST https://your-username-r3m-api.hf.space/process \
  -H "Content-Type: application/json" \
  -d '{"content": "Sample document content"}'
```

## Features

- ✅ **Free hosting** - No cost
- ✅ **Your actual C++ code** - Real performance
- ✅ **Public URL** - Get a link immediately
- ✅ **Auto-restart** - On code changes
- ✅ **Health checks** - Built-in monitoring

## Hardware

- **CPU**: 2 vCPU (free)
- **Memory**: 16GB RAM
- **Storage**: 50GB disk
- **Network**: HTTP/HTTPS ports 80, 443, 8080 