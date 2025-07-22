#!/bin/bash

# GitHub Setup Script for AGL Game Engine
# Run this after creating your GitHub repository

GITHUB_USERNAME="heinsteinh"
REPO_NAME="agl-game-engine"  # Change this if you used a different name

echo "Setting up GitHub remote for AGL Game Engine..."
echo "GitHub User: $GITHUB_USERNAME"
echo "Repository: $REPO_NAME"
echo ""

# Check if remote already exists
if git remote get-url origin >/dev/null 2>&1; then
    echo "Remote 'origin' already exists:"
    git remote get-url origin
    echo ""
    read -p "Do you want to update it? (y/n): " update_remote
    if [[ $update_remote == "y" || $update_remote == "Y" ]]; then
        git remote set-url origin https://github.com/$GITHUB_USERNAME/$REPO_NAME.git
        echo "Remote updated!"
    fi
else
    # Add remote
    git remote add origin https://github.com/$GITHUB_USERNAME/$REPO_NAME.git
    echo "Remote added: https://github.com/$GITHUB_USERNAME/$REPO_NAME.git"
fi

echo ""
echo "Current remotes:"
git remote -v

echo ""
echo "Ready to push to GitHub!"
echo "Run these commands when ready:"
echo ""
echo "  git push -u origin main"
echo ""
echo "Or run with confirmation:"
read -p "Push to GitHub now? (y/n): " push_now

if [[ $push_now == "y" || $push_now == "Y" ]]; then
    echo "Pushing to GitHub..."
    git push -u origin main

    if [ $? -eq 0 ]; then
        echo ""
        echo "🎉 Success! Your project is now on GitHub:"
        echo "   https://github.com/$GITHUB_USERNAME/$REPO_NAME"
        echo ""
        echo "Next steps:"
        echo "1. Visit your repository on GitHub"
        echo "2. Add topics/tags for discoverability"
        echo "3. Consider adding a license file"
        echo "4. Star your own repository! ⭐"
    else
        echo ""
        echo "❌ Push failed. Please check:"
        echo "1. Repository exists on GitHub"
        echo "2. You have push permissions"
        echo "3. Repository name matches: $REPO_NAME"
    fi
else
    echo ""
    echo "When you're ready to push, run:"
    echo "  git push -u origin main"
fi
