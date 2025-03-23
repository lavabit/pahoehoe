# -*- mode: ruby -*-
# vi: set ft=ruby :

Vagrant.configure("2") do |config|

  config.ssh.forward_x11 = true
  config.ssh.forward_agent = true
  pahoehoe_network_name = "pahoehoe_#{SecureRandom.random_number(99999999)}"
  
  config.vm.define "debian_build" do |debian_build|
    debian_build.vm.box = "generic/debian10"
    debian_build.vm.hostname = "debian-build"
    debian_build.vm.boot_timeout = 1200
    debian_build.vm.base_mac = "08002727A0A0"
    debian_build.vm.synced_folder ".", "/vagrant", disabled: true
    debian_build.vm.network "forwarded_port", guest: 22, host: 35000, id: "ssh", auto_correct: true
    debian_build.vm.network :private_network, :ip => "192.168.221.50", :libvirt__network_name => pahoehoe_network_name, :virtualbox__intnet => pahoehoe_network_name
    # debian_build.vm.synced_folder "android/", "/home/vagrant/android/", create: true, disabled: false, type: "nfs", nfs_udp: false, nfs_version: "3", :mount_options => ['nolock,noatime,fsc,ac,actimeo=120,async,retrans=10']
    # :mount_options => ['nolock,vers=3,tcp,noatime,fsc,actimeo=1,async']

    debian_build.vm.provider :libvirt do |v, override|
      v.default_prefix = "proxy_"
      v.driver = "kvm"
      v.nested = true
      v.memory = 12384
      v.cpus = 8
      v.cputopology :sockets => "1", :cores => "4", :threads => "2"
      if Vagrant.has_plugin?( "vagrant-libvirt", ">= 0.7.0" )
        v.disk_bus = "scsi"
        v.disk_driver :cache => "unsafe", :discard => "unmap", :detect_zeroes => "unmap", :io => "threads"
      end

      # v.channel :type => 'unix', :target_name => 'org.qemu.guest_agent.0', :disabled => false
      # For libvirt plugin versions below 0.4.0.
      # v.volume_cache = "unsafe"
      # For libvirt plugin versions equal to, or higher than 0.4.0.
      # v.disk_driver :cache => 'unsafe', :discard => 'unmap', :detect_zeroes => 'unmap'
    end

    debian_build.vm.provider :virtualbox do |v, override|
      v.cpus = 4
      v.memory = 4096
      v.gui = false
      v.customize ["modifyvm", :id, "--pagefusion", "on"]
      # v.customize ["modifyvm", :id, "--guestmemoryballoon", 2048]
      v.customize ["storagectl", :id, "--name", "SATA Controller", "--hostiocache", "on"]
      v.customize ["setextradata", :id, "VBoxInternal/Devices/VMMDev/0/Config/GetHostTimeDisabled", 1]
    end

    debian_build.vm.provision "shell", inline: <<-SHELL
      # set -x
      export DEBIAN_FRONTEND=noninteractive
      export DEBCONF_NONINTERACTIVE_SEEN=true
cat <<-EOF | sudo debconf-set-selections
grub-pc grub-pc/install_devices_disks_changed string $(grub-probe --target=disk /boot)
grub-pc grub-pc/install_devices string $(grub-probe --target=disk /boot)
EOF
      apt-get -qq update && apt-get -qq -y upgrade < /dev/null > /dev/null
      apt-get -qq -y install xauth dbus-x11 < /dev/null > /dev/null
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
  #   ubuntu_aosp.vm.boot_timeout = 1200
  #   ubuntu_aosp.vm.base_mac = "08002727D0D0"
  #   ubuntu_aosp.vm.synced_folder ".", "/vagrant", disabled: true
  #   ubuntu_aosp.vm.network "forwarded_port", guest: 22, host: 34000, id: "ssh", auto_correct: true
  #   ubuntu_aosp.vm.network :private_network, :ip => "192.168.221.51", :libvirt__network_name => pahoehoe_network_name, :virtualbox__intnet => pahoehoe_network_name
  #
  #   ubuntu_aosp.vm.provider :libvirt do |v, override|
  #     v.default_prefix = "proxy_"
  #     v.driver = "kvm"
  #     v.nested = true
  #     v.memory = 12384
  #     v.cpus = 4
  #     v.cputopology :sockets => '1', :cores => '4', :threads => '2'
  #     if Vagrant.has_plugin?( "vagrant-libvirt", ">= 0.7.0" )
  #       v.disk_bus = "scsi"
  #       v.disk_driver :cache => "unsafe", :discard => "unmap", :detect_zeroes => "unmap", :io => "threads"
  #     end
  #     v.channel :type => 'unix', :target_name => 'org.qemu.guest_agent.0', :disabled => false
  #     # For libvirt plugin versions below 0.4.0.
  #     # v.volume_cache = "unsafe"
  #     # For libvirt plugin versions equal to, or higher than 0.4.0.
  #     v.disk_driver :cache => 'unsafe', :discard => 'unmap', :detect_zeroes => 'unmap'
  #   end
  #
  #   ubuntu_aosp.vm.provider :virtualbox do |v, override|
  #     v.cpus = 4
  #     v.memory = 4096
  #     v.gui = false
  #     v.customize ["modifyvm", :id, "--pagefusion", "on"]
  #     v.customize ["storagectl", :id, "--name", "SATA Controller", "--hostiocache", "on"]
  #     v.customize ["setextradata", :id, "VBoxInternal/Devices/VMMDev/0/Config/GetHostTimeDisabled", 1]
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
    debian_vpn.vm.boot_timeout = 1200
    debian_vpn.vm.base_mac = "08002727B0B0"
    debian_vpn.vm.synced_folder ".", "/vagrant", disabled: true
    debian_vpn.vm.network "forwarded_port", guest: 22, host: 31000, id: "ssh", auto_correct: true
    debian_vpn.vm.network :private_network, :ip => "192.168.221.145", :libvirt__network_name => pahoehoe_network_name, :virtualbox__intnet => pahoehoe_network_name
    debian_vpn.vm.network :private_network, :ip => "192.168.221.146", :libvirt__network_name => pahoehoe_network_name, :virtualbox__intnet => pahoehoe_network_name

    debian_vpn.vm.provider :libvirt do |v, override|
      v.driver = "kvm"
      v.nested = true
      v.memory = 1024
      v.cpus = 1
      v.default_prefix = "proxy_"      
      if Vagrant.has_plugin?( "vagrant-libvirt", ">= 0.7.0" )
        v.disk_bus = "scsi"
        v.disk_driver :cache => "unsafe", :discard => "unmap", :detect_zeroes => "unmap", :io => "threads"
      end

      # v.channel :type => 'unix', :target_name => 'org.qemu.guest_agent.0', :disabled => true
      # For libvirt plugin versions below 0.4.0.
      # v.volume_cache = "unsafe"
      # For libvirt plugin versions equal to, or higher than 0.4.0.
      # v.disk_driver :cache => 'unsafe', :discard => 'unmap', :detect_zeroes => 'unmap'
    end

    debian_vpn.vm.provider :virtualbox do |v, override|
      v.cpus = 1
      v.memory = 1024
      v.gui = false
      v.customize ["modifyvm", :id, "--pagefusion", "on"]
      # v.customize ["modifyvm", :id, "--guestmemoryballoon", 512]
      v.customize ["storagectl", :id, "--name", "SATA Controller", "--hostiocache", "on"]
      v.customize ["setextradata", :id, "VBoxInternal/Devices/VMMDev/0/Config/GetHostTimeDisabled", 1]
    end

    debian_vpn.vm.provision "shell", inline: <<-SHELL
      # set -x
      export DEBIAN_FRONTEND=noninteractive
      export DEBCONF_NONINTERACTIVE_SEEN=true
cat <<-EOF | sudo debconf-set-selections
grub-pc grub-pc/install_devices_disks_changed string $(grub-probe --target=disk /boot)
grub-pc grub-pc/install_devices string $(grub-probe --target=disk /boot)
EOF
      apt-get -qq update &> /dev/null && apt-get -qq -y upgrade < /dev/null > /dev/null
      apt-get -qq -y install xauth dbus-x11 < /dev/null > /dev/null
      echo "* * * * * root command bash -c '/etc/cron.daily/mlocate'" > /etc/cron.d/updatedb
      sudo sed -i 's/.*X11Forwarding.*/X11Forwarding yes/g' /etc/ssh/sshd_config
      sudo sed -i 's/.*X11UseLocalhost.*/X11UseLocalhost no/g' /etc/ssh/sshd_config
      sudo sed -i 's/.*X11DisplayOffset.*/X11DisplayOffset 10/g' /etc/ssh/sshd_config
      sudo systemctl restart sshd.service 2> /dev/null
    SHELL

  end

  config.vm.define "alma_vpn" do |alma_vpn|
    alma_vpn.vm.box = "generic/alma9"
    alma_vpn.vm.hostname = "alma-vpn"
    alma_vpn.vm.boot_timeout = 1200
    alma_vpn.vm.base_mac = "08002727C0C0"
    alma_vpn.vm.synced_folder ".", "/vagrant", disabled: true
    alma_vpn.vm.network "forwarded_port", guest: 22, host: 30000, id: "ssh", auto_correct: true
    alma_vpn.vm.network :private_network, :ip => "192.168.221.245", :libvirt__network_name => pahoehoe_network_name, :virtualbox__intnet => pahoehoe_network_name
    alma_vpn.vm.network :private_network, :ip => "192.168.221.246", :libvirt__network_name => pahoehoe_network_name, :virtualbox__intnet => pahoehoe_network_name

    alma_vpn.vm.provider :libvirt do |v, override|
      v.driver = "kvm"
      v.nested = true
      v.memory = 1024
      v.cpus = 1
      v.default_prefix = "proxy_"
      if Vagrant.has_plugin?( "vagrant-libvirt", ">= 0.7.0" )
        v.disk_bus = "scsi"
        v.disk_driver :cache => "unsafe", :discard => "unmap", :detect_zeroes => "unmap", :io => "threads"
      end

      # v.channel :type => 'unix', :target_name => 'org.qemu.guest_agent.0', :disabled => true
      # For libvirt plugin versions below 0.4.0.
      # v.volume_cache = "unsafe"
      # For libvirt plugin versions equal to, or higher than 0.4.0.
      # v.disk_driver :cache => 'unsafe', :discard => 'unmap', :detect_zeroes => 'unmap'
    end

    alma_vpn.vm.provider :virtualbox do |v, override|
      v.cpus = 1
      v.memory = 1024
      v.gui = false
      v.customize ["modifyvm", :id, "--pagefusion", "on"]
      # v.customize ["modifyvm", :id, "--guestmemoryballoon", 512]
      v.customize ["storagectl", :id, "--name", "SATA Controller", "--hostiocache", "on"]
      v.customize ["setextradata", :id, "VBoxInternal/Devices/VMMDev/0/Config/GetHostTimeDisabled", 1]
    end

    alma_vpn.vm.provision "shell", inline: <<-SHELL
      # set -x
      dnf -q -y update 2> /dev/null
      dnf -q -y install xauth dbus-x11 2> /dev/null
      echo "* * * * * root command bash -c '/usr/bin/updatedb'" > /etc/cron.d/updatedb
      sudo sed -i 's/.*X11Forwarding.*/X11Forwarding yes/g' /etc/ssh/sshd_config
      sudo sed -i 's/.*X11UseLocalhost.*/X11UseLocalhost no/g' /etc/ssh/sshd_config
      sudo sed -i 's/.*X11DisplayOffset.*/X11DisplayOffset 10/g' /etc/ssh/sshd_config
      # We need to set the crypto policy to LEGACY for the CI server.
      sudo update-crypto-policies --set LEGACY
      sudo systemctl restart sshd.service 2> /dev/null
    SHELL

  end

end
