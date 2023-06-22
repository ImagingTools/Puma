# Root of Puma project
TEMPLATE = subdirs

SUBDIRS += pumagql
pumagql.file = ../../Include/pumagql/QMake/pumagql.pro

SUBDIRS += PumaGqlPck
PumaGqlPck.file = ../../Impl/PumaGqlPck/QMake/PumaGqlPck.pro
PumaGqlPck.depends = pumagql

SUBDIRS += PumaServer
PumaServer.file = ../../Impl/PumaServer/QMake/PumaServer.pro
PumaServer.depends = PumaGqlPck

