# Use an official base image
FROM ubuntu:20.04

# Install any dependencies
RUN apt-get update && apt-get install -y \
    software-properties-common \
    && add-apt-repository ppa:bibledit/ppa \
    && apt-get update \
    && apt-get install -y bibledit

# Expose the port Bibledit runs on
EXPOSE 8080

# Define the command to run the app
CMD ["bibledit"]
