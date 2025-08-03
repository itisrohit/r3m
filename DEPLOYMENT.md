# R3M API Deployment Guide

This guide covers deploying the R3M Document Processing API to Fly.io, a free and reliable platform for containerized applications.

## 🚀 Quick Deploy to Fly.io

### Prerequisites
- GitHub account
- Fly.io account (free)

### Step 1: Create Fly.io Account
1. **Visit [fly.io](https://fly.io)**
2. **Sign up** with your GitHub account
3. **Install Fly CLI** (for local setup):
   ```bash
   curl -L https://fly.io/install.sh | sh
   ```

### Step 2: Create Fly.io App (Manual Setup)
1. **Clone your repository locally:**
   ```bash
   git clone https://github.com/itisrohit/r3m.git
   cd r3m
   ```

2. **Login to Fly.io:**
   ```bash
   fly auth login
   ```

3. **Create the app (this will create fly.toml):**
   ```bash
   fly launch --no-deploy
   ```
   - **App name:** `r3m-api`
   - **Region:** Choose closest to you
   - **Confirm settings:** Yes

4. **Push the fly.toml file:**
   ```bash
   git add fly.toml
   git commit -m "Add Fly.io configuration"
   git push origin testings2
   ```

### Step 3: Get Fly.io API Token
1. **Create a deploy token:**
   ```bash
   fly tokens create deploy -x 999999h
   ```
2. **Copy the entire token** (including `FlyV1` and space at the beginning)

### Step 4: Add Token to GitHub Secrets
1. **Go to your GitHub repository**
2. **Settings** → **Secrets and variables** → **Actions**
3. **Add new repository secret:**
   - **Name:** `FLY_API_TOKEN`
   - **Value:** Your Fly.io deploy token (including `FlyV1` prefix)

### Step 5: Deploy Automatically
1. **Push to `testings2` branch**
2. **GitHub Actions will automatically:**
   - Build your Docker image
   - Deploy to Fly.io
   - Make your API available at `https://r3m-api.fly.dev`

## 🔧 Configuration

### Environment Variables
The following environment variables are automatically set in `fly.toml`:
```toml
R3M_ENABLE_MEMORY_POOLING = "false"
R3M_MAX_WORKERS = "2"
R3M_OPTIMAL_BATCH_SIZE = "8"
PORT = "8080"
```

### Resource Limits
- **CPU:** 1 shared CPU
- **Memory:** 256 MB
- **Storage:** 3 GB persistent volume
- **Bandwidth:** 160 GB/month

### Health Checks
- **Path:** `/health`
- **Interval:** 30 seconds
- **Timeout:** 5 seconds
- **Grace Period:** 10 seconds

## 🌐 Access Your API

### Endpoints
- **Health Check:** `https://r3m-api.fly.dev/health`
- **API Info:** `https://r3m-api.fly.dev/info`
- **Document Processing:** `https://r3m-api.fly.dev/process`

### Custom Domain (Optional)
1. **Go to Fly.io Dashboard**
2. **Select your app**
3. **Go to Domains**
4. **Add your custom domain**

## 📊 Monitoring

### Fly.io Dashboard
- **Real-time logs**
- **Resource usage**
- **Deployment status**
- **Error tracking**

### GitHub Actions
- **Build status**
- **Deployment logs**
- **Automatic rollbacks**

## 🔄 CI/CD Pipeline

### Automatic Deployment
- **Trigger:** Push to `testings2` branch
- **Build:** Docker image from your `Dockerfile`
- **Deploy:** To Fly.io with zero downtime
- **Health Check:** Automatic verification

### Manual Deployment
```bash
# Install Fly CLI
curl -L https://fly.io/install.sh | sh

# Login to Fly.io
fly auth login

# Deploy manually
fly deploy
```

## 🛠️ Troubleshooting

### Common Issues

#### App Not Found Error
- **Solution:** Create the app manually using `fly launch --no-deploy`
- **Steps:** 
  1. Clone repo locally
  2. Run `fly launch --no-deploy`
  3. Push the generated `fly.toml` file

#### Build Failures
- **Check:** Dockerfile syntax
- **Verify:** All dependencies are included
- **Review:** Build logs in GitHub Actions

#### Runtime Errors
- **Check:** Application logs in Fly.io dashboard
- **Verify:** Environment variables are set correctly
- **Test:** Health endpoint manually

#### Memory Issues
- **Increase:** Memory limit in `fly.toml`
- **Optimize:** Application memory usage
- **Monitor:** Resource usage in dashboard

### Debug Commands
```bash
# View app logs
fly logs

# SSH into app
fly ssh console

# Check app status
fly status

# Scale app
fly scale count 1
```

## 💰 Cost Analysis

### Free Tier Limits
- **3 shared-cpu-1x 256mb VMs** (always running)
- **160 GB bandwidth** per month
- **3GB persistent volume storage**
- **Perfect for:** Small to medium APIs

### Usage Monitoring
- **Fly.io Dashboard:** Real-time usage
- **GitHub Actions:** Build and deploy costs
- **Alerts:** Automatic notifications for limits

## 🚀 Performance Optimization

### For R3M API
- **Memory Pooling:** Disabled for stability
- **Worker Threads:** Limited to 2 for free tier
- **Batch Size:** Optimized to 8 for efficiency
- **Health Checks:** Regular monitoring

### Scaling Options
- **Auto-scaling:** Available on paid plans
- **Manual scaling:** `fly scale count 2`
- **Resource limits:** Adjustable in `fly.toml`

## 📝 Next Steps

1. **Create App:** Run `fly launch --no-deploy` locally
2. **Add Token:** To GitHub secrets
3. **Deploy:** Push to `testings2` branch
4. **Test:** Verify API endpoints work
5. **Monitor:** Check logs and performance
6. **Scale:** Upgrade if needed

Your R3M API will be live at `https://r3m-api.fly.dev`! 🎉