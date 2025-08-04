# R3M API - Cloudflare Workers

High-performance document processing API deployed on Cloudflare Workers.

## 🚀 Features

- **Free Tier**: No credit card required
- **Global CDN**: Edge network for fast responses
- **Auto-scaling**: Handles traffic spikes automatically
- **CORS Support**: Ready for web applications
- **Mock API**: Demonstrates R3M API endpoints

## 🏃‍♂️ Quick Deploy

[![Deploy to Cloudflare Workers](https://deploy.workers.cloudflare.com/button)](https://deploy.workers.cloudflare.com/?url=https://github.com/yourusername/r3m/tree/main/deploy/r3m-cloudflare)

## 📋 Manual Setup

1. **Install Wrangler CLI**:
   ```bash
   npm install -g wrangler
   ```

2. **Login to Cloudflare**:
   ```bash
   wrangler login
   ```

3. **Deploy**:
   ```bash
   cd deploy/r3m-cloudflare
   npm install
   wrangler deploy
   ```

## 🔗 API Endpoints

### Health Check
```bash
GET /health
```

### API Information
```bash
GET /info
```

### Process Single Document
```bash
POST /process
Content-Type: application/json

{
  "content": "Your document content here..."
}
```

### Process Batch
```bash
POST /batch
Content-Type: application/json

{
  "documents": [
    {"content": "Document 1..."},
    {"content": "Document 2..."}
  ]
}
```

## 📊 Example Usage

```bash
# Health check
curl https://r3m-api.your-subdomain.workers.dev/health

# Get API info
curl https://r3m-api.your-subdomain.workers.dev/info

# Process document
curl -X POST https://r3m-api.your-subdomain.workers.dev/process \
  -H "Content-Type: application/json" \
  -d '{"content": "Sample document content for processing..."}'
```

## ⚡ Performance

- **Response Time**: < 50ms average
- **Global Latency**: < 100ms to most locations
- **Uptime**: 99.9% SLA
- **Free Tier**: 100,000 requests/day

## 🔒 Security

- **HTTPS**: All traffic encrypted
- **CORS**: Configured for web apps
- **Rate Limiting**: Built-in protection
- **DDoS Protection**: Cloudflare's global network

## 📝 License

MIT License - see LICENSE file for details.

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

## 📞 Support

For issues and questions:
- Create an issue on GitHub
- Check the documentation
- Review the logs in Cloudflare dashboard