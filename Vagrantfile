# -*- mode: ruby -*-
# vi: set ft=ruby :

Vagrant.configure("2") do |config|

  config.ssh.forward_x11 = true
  config.ssh.forward_agent = true

  config.vm.define "debian_build" do |debian_build|
    debian_build.vm.box = "generic/debian10"
    debian_build.vm.hostname = "debian-build"
    debian_build.vm.network :private_network, :ip => "192.168.221.50"
    # debian_build.vm.synced_folder "android/", "/home/vagrant/android/", create: true, disabled: false, type: "nfs", nfs_udp: false, nfs_version: "3", :mount_options => ['nolock,noatime,fsc,ac,actimeo=120,async,retrans=10']
    # :mount_options => ['nolock,vers=3,tcp,noatime,fsc,actimeo=1,async']

    debian_build.vm.provider :libvirt do |v, override|
      v.default_prefix = "proxy_"
      v.driver = "kvm"
      v.nested = true
      v.memory = 12384
      v.cpus = 8
      v.cputopology :sockets => '1', :cores => '4', :threads => '2'
      # For libvirt plugin versions below 0.4.0.
      # v.volume_cache = "unsafe"
      # For libvirt plugin versions equal to, or higher than 0.4.0.
      v.disk_driver :cache => 'unsafe', :discard => 'unmap', :detect_zeroes => 'unmap'
    end

    debian_build.vm.provider :virtualbox do |v, override|
      v.gui = false
      v.customize ["modifyvm", :id, "--memory", 8192]
      v.customize ["modifyvm", :id, "--cpus", 4]
    end

    debian_build.vm.provision "shell", inline: <<-SHELL
      # set -x
      export DEBIAN_FRONTEND=noninteractive
      export DEBCONF_NONINTERACTIVE_SEEN=true
      apt-get -qq update && apt-get -qq -y upgrade < /dev/null > /dev/null
      echo "* * * * * root command bash -c '/etc/cron.daily/mlocate'" > /etc/cron.d/updatedb
      sudo sed -i 's/.*X11Forwarding.*/X11Forwarding yes/g' /etc/ssh/sshd_config
      sudo sed -i 's/.*X11UseLocalhost.*/X11UseLocalhost no/g' /etc/ssh/sshd_config
      sudo sed -i 's/.*X11DisplayOffset.*/X11DisplayOffset 10/g' /etc/ssh/sshd_config
      sudo systemctl restart sshd.service
    SHELL

  end

  # config.vm.define "ubuntu_aosp" do |debian_build|
  #   ubuntu_aosp.vm.box = "generic/ubuntu1804"
  #   ubuntu_aosp.vm.hostname = "ubuntu-aosp"
  #   ubuntu_aosp.vm.network :private_network, :ip => "192.168.221.51"
  #
  #   ubuntu_aosp.vm.provider :libvirt do |v, override|
  #     v.driver = "kvm"
  #     v.nested = true
  #     v.memory = 12384
  #     v.cpus = 4
  #     v.default_prefix = "proxy_"
  #     # For libvirt plugin versions below 0.4.0.
  #     # v.volume_cache = "unsafe"
  #     # For libvirt plugin versions equal to, or higher than 0.4.0.
  #     v.disk_driver :cache => 'unsafe', :discard => 'unmap', :detect_zeroes => 'unmap'
  #   end
  #
  #   ubuntu_aosp.vm.provider :virtualbox do |v, override|
  #     v.gui = false
  #     v.customize ["modifyvm", :id, "--memory", 12384]
  #     v.customize ["modifyvm", :id, "--cpus", 4]
  # =>  end
  #
  #   ubuntu_aosp.vm.provision "shell", inline: <<-SHELL
  #     # set -x
  #     export DEBIAN_FRONTEND=noninteractive
  #     export DEBCONF_NONINTERACTIVE_SEEN=true
  #     apt-get -qq update && apt-get -qq -y upgrade < /dev/null > /dev/null
  #     echo "* * * * * root command bash -c '/etc/cron.daily/mlocate'" > /etc/cron.d/updatedb
  #     sudo sed -i 's/.*X11Forwarding.*/X11Forwarding yes/g' /etc/ssh/sshd_config
  #     sudo sed -i 's/.*X11UseLocalhost.*/X11UseLocalhost no/g' /etc/ssh/sshd_config
  #     sudo sed -i 's/.*X11DisplayOffset.*/X11DisplayOffset 10/g' /etc/ssh/sshd_config
  #     sudo systemctl restart sshd.service
  #   SHELL
  # end

  config.vm.define "debian_vpn" do |debian_vpn|
    debian_vpn.vm.box = "generic/debian10"
    debian_vpn.vm.hostname = "debian-vpn"
    debian_vpn.vm.network :private_network, :ip => "192.168.221.142"
    debian_vpn.vm.network :private_network, :ip => "192.168.221.143"
    debian_vpn.vm.network :private_network, :ip => "192.168.221.144"
    debian_vpn.vm.network :private_network, :ip => "192.168.221.145"
    debian_vpn.vm.network :private_network, :ip => "192.168.221.146"

    debian_vpn.vm.provider :libvirt do |v, override|
      v.driver = "kvm"
      v.nested = true
      v.memory = 1024
      v.cpus = 1
      v.default_prefix = "proxy_"
      # For libvirt plugin versions below 0.4.0.
      # v.volume_cache = "unsafe"
      # For libvirt plugin versions equal to, or higher than 0.4.0.
      v.disk_driver :cache => 'unsafe', :discard => 'unmap', :detect_zeroes => 'unmap'
    end

    debian_vpn.vm.provider :virtualbox do |v, override|
      v.gui = false
      v.customize ["modifyvm", :id, "--memory", 1024]
      v.customize ["modifyvm", :id, "--cpus", 1]
    end

    debian_vpn.vm.provision "shell", inline: <<-SHELL
      # set -x
      export DEBIAN_FRONTEND=noninteractive
      export DEBCONF_NONINTERACTIVE_SEEN=true
      apt-get -qq update && apt-get -qq -y upgrade < /dev/null > /dev/null
      echo "* * * * * root command bash -c '/etc/cron.daily/mlocate'" > /etc/cron.d/updatedb
      sudo sed -i 's/.*X11Forwarding.*/X11Forwarding yes/g' /etc/ssh/sshd_config
      sudo sed -i 's/.*X11UseLocalhost.*/X11UseLocalhost no/g' /etc/ssh/sshd_config
      sudo sed -i 's/.*X11DisplayOffset.*/X11DisplayOffset 10/g' /etc/ssh/sshd_config
      sudo systemctl restart sshd.service
    SHELL

  end

  config.vm.define "centos_vpn" do |centos_vpn|
    centos_vpn.vm.box = "generic/centos8"
    centos_vpn.vm.hostname = "centos-vpn"
    centos_vpn.vm.network :private_network, :ip => "192.168.221.242"
    centos_vpn.vm.network :private_network, :ip => "192.168.221.243"
    centos_vpn.vm.network :private_network, :ip => "192.168.221.244"
    centos_vpn.vm.network :private_network, :ip => "192.168.221.245"
    centos_vpn.vm.network :private_network, :ip => "192.168.221.246"

    centos_vpn.vm.provider :libvirt do |v, override|
      v.driver = "kvm"
      v.nested = true
      v.memory = 1024
      v.cpus = 1
      v.default_prefix = "proxy_"
      # For libvirt plugin versions below 0.4.0.
      # v.volume_cache = "unsafe"
      # For libvirt plugin versions equal to, or higher than 0.4.0.
      v.disk_driver :cache => 'unsafe', :discard => 'unmap', :detect_zeroes => 'unmap'
    end

    centos_vpn.vm.provider :virtualbox do |v, override|
      v.gui = false
      v.customize ["modifyvm", :id, "--memory", 1024]
      v.customize ["modifyvm", :id, "--cpus", 1]
    end

    centos_vpn.vm.provision "shell", inline: <<-SHELL
      # set -x
      dnf -q -y update
      echo "* * * * * root command bash -c '/usr/bin/updatedb'" > /etc/cron.d/updatedb
      sudo sed -i 's/.*X11Forwarding.*/X11Forwarding yes/g' /etc/ssh/sshd_config
      sudo sed -i 's/.*X11UseLocalhost.*/X11UseLocalhost no/g' /etc/ssh/sshd_config
      sudo sed -i 's/.*X11DisplayOffset.*/X11DisplayOffset 10/g' /etc/ssh/sshd_config
      sudo systemctl restart sshd.service
    SHELL

  end

end
