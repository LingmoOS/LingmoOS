// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "lsysinfo.h"

#include <QCoreApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QThread>
#include <QFile>

#include <stdio.h>

DCORE_USE_NAMESPACE

bool distributionInfoValid() {
    return QFile::exists(LSysInfo::distributionInfoPath());
}

void printDistributionOrgInfo(LSysInfo::OrgType type) {
    QString sectionName = LSysInfo::distributionInfoSectionName(type);
    printf("%s Name: %s\n", qPrintable(sectionName), qPrintable(LSysInfo::distributionOrgName(type)));
    printf("%s Logo (Normal size): %s\n", qPrintable(sectionName), qPrintable(LSysInfo::distributionOrgLogo(type)));
    printf("%s Website: %s\n", qPrintable(sectionName), qPrintable(LSysInfo::distributionOrgWebsite(type).second));
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    Q_UNUSED(app)

    QCommandLineParser parser;
    QCommandLineOption option_all("all", "Print All Information");
    QCommandLineOption option_lingmo_type("lingmo-type", " ");
    QCommandLineOption option_lingmo_version("lingmo-version", " ");
    QCommandLineOption option_lingmo_edition("lingmo-edition", " ");
    QCommandLineOption option_lingmo_copyright("lingmo-copyright", " ");
    QCommandLineOption option_product_type("product-type", " ");
    QCommandLineOption option_product_version("product-version", " ");
    QCommandLineOption option_computer_name("computer-name", "Computer Name");
    QCommandLineOption option_cpu_model("cpu-model", "CPU Model");
    QCommandLineOption option_installed_memory_size("installed-memory-size", "Installed Memory Size (GiB)");
    QCommandLineOption option_memory_size("memory-size", "Memory Size (GiB)");
    QCommandLineOption option_disk_size("disk-size", "Disk Size (GiB)");
    QCommandLineOption option_distribution_info("distribution-info", "Distribution information");
    QCommandLineOption option_distributer_info("distributer-info", "Distributer information");

    parser.addOptions({option_all, option_lingmo_type, option_lingmo_version, option_lingmo_edition,
                       option_lingmo_copyright, option_product_type, option_product_version,
                       option_computer_name, option_cpu_model, option_installed_memory_size, option_memory_size,
                       option_disk_size, option_distribution_info, option_distributer_info});
    parser.addHelpOption();
    parser.addVersionOption();
    parser.process(app);

    if (argc < 2)
        parser.showHelp();

    if (parser.isSet(option_all)) {
        printf("Computer Name: %s\n", qPrintable(LSysInfo::computerName()));
        printf("CPU Model: %s x %d\n", qPrintable(LSysInfo::cpuModelName()), QThread::idealThreadCount());
        printf("Installed Memory Size: %f GiB\n", LSysInfo::memoryInstalledSize() / 1024.0 / 1024 / 1024);
        printf("Memory Size: %f GiB\n", LSysInfo::memoryTotalSize() / 1024.0 / 1024 / 1024);
        printf("Disk Size: %f GiB\n", LSysInfo::systemDiskSize() / 1024.0 / 1024 / 1024);

        if (LSysInfo::isLingmo() && LSysInfo::isOCEAN()) {
            printf("Lingmo Type: %s\n", qPrintable(LSysInfo::lingmoTypeDisplayName()));
            printf("Lingmo Version: %s\n", qPrintable(LSysInfo::lingmoVersion()));

            if (!LSysInfo::lingmoEdition().isEmpty())
                printf("Lingmo Edition: %s\n", qPrintable(LSysInfo::lingmoEdition()));

            if (!LSysInfo::lingmoCopyright().isEmpty())
                printf("Lingmo Copyright: %s\n", qPrintable(LSysInfo::lingmoCopyright()));
        }

        printf("Operating System Name: %s\n", qPrintable(LSysInfo::operatingSystemName()));
        printf("Product Type: %s\n", qPrintable(LSysInfo::productTypeString()));
        printf("Product Version: %s\n", qPrintable(LSysInfo::productVersion()));

        if (LSysInfo::isLingmo()) {
            printf("Uos Product Name: %s\n", qPrintable(LSysInfo::uosProductTypeName()));
            printf("Uos SystemName Name: %s\n", qPrintable(LSysInfo::uosSystemName()));
            printf("Uos Edition Name: %s\n", qPrintable(LSysInfo::uosEditionName()));
            printf("Uos SP Version: %s\n", qPrintable(LSysInfo::spVersion()));
            printf("Uos update Version: %s\n", qPrintable(LSysInfo::udpateVersion()));
            printf("Uos major Version: %s\n", qPrintable(LSysInfo::majorVersion()));
            printf("Uos minor Version: %s\n", qPrintable(LSysInfo::minorVersion()));
            printf("Uos build Version: %s\n", qPrintable(LSysInfo::buildVersion()));
        }
        if (distributionInfoValid()) {
            printDistributionOrgInfo(LSysInfo::Distribution);
            printDistributionOrgInfo(LSysInfo::Distributor);
        }
    } else {
        if (parser.isSet(option_lingmo_type))
            printf("%s", qPrintable(LSysInfo::uosEditionName(QLocale::c())));
        else if (parser.isSet(option_lingmo_version))
            printf("%s", qPrintable(LSysInfo::majorVersion()));
        else if (parser.isSet(option_lingmo_edition))
            printf("%s", qPrintable(LSysInfo::lingmoEdition()));
        else if (parser.isSet(option_lingmo_copyright))
            printf("%s", qPrintable(LSysInfo::lingmoCopyright()));
        else if (parser.isSet(option_product_type))
            printf("%s", qPrintable(LSysInfo::productTypeString()));
        else if (parser.isSet(option_product_version))
            printf("%s", qPrintable(LSysInfo::productVersion()));
        else if (parser.isSet(option_cpu_model))
            printf("%s x %d", qPrintable(LSysInfo::cpuModelName()), QThread::idealThreadCount());
        else if (parser.isSet(option_computer_name))
            printf("%s", qPrintable(LSysInfo::computerName()));
        else if (parser.isSet(option_installed_memory_size))
            printf("%f", LSysInfo::memoryInstalledSize() / 1024.0 / 1024 / 1024);
        else if (parser.isSet(option_memory_size))
            printf("%f", LSysInfo::memoryTotalSize() / 1024.0 / 1024 / 1024);
        else if (parser.isSet(option_disk_size))
            printf("%f", LSysInfo::systemDiskSize() / 1024.0 / 1024 / 1024);
        else if (parser.isSet(option_distribution_info)) {
            printDistributionOrgInfo(LSysInfo::Distribution);
        } else if (parser.isSet(option_distributer_info)) {
            printDistributionOrgInfo(LSysInfo::Distributor);
        }
    }

    return 0;
}
