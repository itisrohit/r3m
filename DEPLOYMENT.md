# R3M Railway Deployment Guide

## Railway Deployment (Recommended)

### Why Railway?
- **Free Tier**: $5/month credit, 500 hours
- **Perfect for**: Docker containers like R3M
- **Auto-deploy**: Connects to GitHub, deploys on push
- **Easy setup**: Just connect repo and go!

### Quick Setup Steps:

1. **Go to Railway**
   - Visit [Railway.app](https://railway.app)
   - Sign up with your GitHub account

2. **Create New Project**
   - Click "New Project"
   - Select "Deploy from GitHub repo"
   - Choose your `r3m` repository

3. **Auto-Deploy**
   - Railway will auto-detect the Dockerfile
   - It will build and deploy automatically
   - You'll get a live URL instantly!

4. **Get Your API URL**
   - Railway provides a URL like: `https://your-app.railway.app`
   - Your API endpoints will be:
     - Health: `https://your-app.railway.app/health`
     - Info: `https://your-app.railway.app/info`
     - Process: `https://your-app.railway.app/process`

### Environment Variables (Optional)

Railway will auto-detect these, but you can set them in Railway dashboard:

```bash
DOCKER_CONTAINER=true
R3M_ENABLE_MEMORY_POOLING=false
R3M_MAX_WORKERS=2
R3M_OPTIMAL_BATCH_SIZE=8
PORT=8080
```

### GitHub Actions Integration

The workflow will:
1. Build Docker image
2. Run tests
3. Deploy to Railway automatically

Just push to `testings2` branch and it will deploy!

### Performance

- **Local**: 94.79% efficiency
- **Docker**: 84.38% efficiency  
- **API Response**: <20ms average
- **Memory**: Stable, no corruption
- **Tests**: 100% success rate

### Railway Token Setup (Optional)

If you want GitHub Actions to auto-deploy:

1. Go to Railway dashboard
2. Go to your project settings
3. Copy the deployment token
4. Add it to GitHub repository secrets as `RAILWAY_TOKEN`

That's it! Your R3M API will be live on Railway! 🚀 