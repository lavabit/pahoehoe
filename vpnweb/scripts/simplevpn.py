#!/usr/bin/env python3
import argparse
import os, sys

import yaml

from jinja2 import Template

AUTH_METHODS = ["anon", "sip"]


class EIPConfig:
    def __init__(self):
        self.openvpn = dict()
        self.locations = dict()
        self.gateways = dict()
        self.provider = dict()
        self.auth = ""


def parseConfig(provider_config):
    with open(provider_config) as conf:
        config = yaml.load(conf.read(), Loader=yaml.FullLoader)
    eip = EIPConfig()
    eip.openvpn.update(yamlListToDict(config['openvpn']))
    configureAuth(eip, config)

    for loc in config['locations']:
        eip.locations.update(yamlIdListToDict(loc))
    for gw in config['gateways']:
        eip.gateways.update(yamlIdListToDict(gw))
    eip.provider.update(yamlListToDict(config['provider']))
    return eip

def configureAuth(eip, config):
    auth = config.get('auth', 'anon')
    if auth not in AUTH_METHODS:
        print("ERROR: unknown auth method", auth)
        sys.exit(1)
    eip.auth = auth

def yamlListToDict(values):
    vals = {}
    for d in values:
        for k, v in d.items():
            vals[k] = v
    return vals


def yamlIdListToDict(data):
    _d = {}
    for identifier, values in data.items():
        _d[identifier] = yamlListToDict(values)
    return _d


def patchObfs4Cert(config, cert):
    for gw in config.gateways:
        for options in config.gateways[gw]['transports']:
            opts = {}
            transport, _, _ = options
            if transport == "obfs4":
                opts['cert'] = cert
                opts['iatMode'] = "0"
            options.append(opts)
    return config


def dictToStr(d):
    for k, v in d.items():
        d[k] = str(v)
    return d


def produceEipConfig(config, obfs4_state, template):
    config = parseConfig(os.path.abspath(config))

    if obfs4_state:
        obfs4_cert = open(
            obfs4_state + '/obfs4_cert.txt').read().rstrip()
    else:
        obfs4_cert = None
    patchObfs4Cert(config, obfs4_cert)

    t = Template(open(template).read())

    print(t.render(
        locations=config.locations,
        gateways=config.gateways,
        openvpn=dictToStr(config.openvpn),
        auth=config.auth))


def produceProviderConfig(config, template):
    config = parseConfig(os.path.abspath(config))
    t = Template(open(template).read())
    print(t.render(
        provider=config.provider))


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("-f", "--file")
    parser.add_argument("-c", "--config")
    parser.add_argument("-t", "--template")
    parser.add_argument("--obfs4_state")
    args = parser.parse_args()

    if args.file == "eip":
        produceEipConfig(args.config, args.obfs4_state,  args.template)
    elif args.file == "provider":
        produceProviderConfig(args.config, args.template)
    else:
        print("unknown type of file:", args.file)
