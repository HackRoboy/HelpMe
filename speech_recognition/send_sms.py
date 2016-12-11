# Download the twilio-python library from http://twilio.com/docs/libraries
from twilio.rest import TwilioRestClient

# Find these values at https://twilio.com/user/account
account_sid = "ACbb72372655bc616a46a24373280de907"  # these are credentials are confidential
auth_token = "7d894b9be0250f232d5b2b274a2e0234"     # these are credentials are confidential
client = TwilioRestClient(account_sid, auth_token)

message = client.messages.create(to="+4915773496762", from_="+4915735986687", body="Your friend/relative is in need of help")##these are credentials are confidential