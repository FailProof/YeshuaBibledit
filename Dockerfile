# Use an official Ubuntu base image
FROM ubuntu:20.04

# Avoid prompts from apt
ENV DEBIAN_FRONTEND=noninteractive

# Install dependencies and add Bibledit repository
RUN apt-get update && apt-get install -y \
    software-properties-common \
    && add-apt-repository ppa:bibledit/ppa \
    && apt-get update \
    && apt-get install -y bibledit-cloud

# Expose the port Bibledit Cloud runs on
EXPOSE 80

# Start Bibledit Cloud
CMD ["bibledit-cloud"]
