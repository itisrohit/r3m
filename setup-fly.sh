#!/bin/bash

echo "🚀 Setting up Fly.io deployment for R3M API"
echo "=============================================="

# Check if flyctl is installed
if ! command -v flyctl &> /dev/null; then
    echo "📦 Installing Fly CLI..."
    curl -L https://fly.io/install.sh | sh
    echo "✅ Fly CLI installed"
else
    echo "✅ Fly CLI already installed"
fi

# Check if user is logged in
echo "🔐 Checking Fly.io authentication..."
if ! flyctl auth whoami &> /dev/null; then
    echo "⚠️  Not logged in to Fly.io"
    echo "🔑 Please login to Fly.io:"
    flyctl auth login
else
    echo "✅ Already logged in to Fly.io"
fi

# Create the app
echo "🏗️  Creating Fly.io app..."
echo "📝 This will create a new app named 'r3m-api'"
echo "🌍 You'll be prompted to choose a region"
echo ""

# Run fly launch with no deploy
fly launch --no-deploy

echo ""
echo "✅ App created successfully!"
echo ""
echo "🔑 Next steps:"
echo "1. Create a deploy token: fly tokens create deploy -x 999999h"
echo "2. Add the token to GitHub secrets as FLY_API_TOKEN"
echo "3. Push to testings2 branch to trigger deployment"
echo ""
echo "📁 The fly.toml file has been created and configured for your app." 