# -*- coding: utf-8 -*-

# Copyright 2011 Éwerton Assis
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from crawler import http
from crawler.rest.unicode import become_unicode

class BaseHandler(object):
    def __init__(self):
        pass

    def get_response(self, request):
        pass

    def __call__(self, raw_data):
        try:
            request = http.Request(raw_data)
        except:
            response = http.responses.error.HttpResponseBadRequest()
        else:
            response = get_response(request)
        return become_unicode(response.writable())

    def load_middleware(self):
        pass

    def handle_exceptions(self, request, controller, info):
        pass

